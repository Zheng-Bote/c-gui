/**
 * SPDX-FileComment: MainWindow implementation for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file MainWindow.cpp
 * @brief Implementation of the main GUI frame.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#include "MainWindow.hpp"
#include "AppController.hpp"
#include "rz_config.hpp"
#include <check_gh-update.hpp>
#include <wx/filedlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace cgui {

enum {
  ID_TOPIC_CHOICE = wxID_HIGHEST + 1,
  ID_INTERFACE_CHOICE,
  ID_LOAD_DATA,
  ID_VALIDATE,
  ID_UPLOAD,
  ID_MANAGE_PLUGINS
};

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(wxID_EXIT, MainWindow::on_exit) 
    EVT_MENU(wxID_ABOUT, MainWindow::on_about)
    EVT_MENU(ID_MANAGE_PLUGINS, MainWindow::on_manage_plugins)
        EVT_COMBOBOX(ID_TOPIC_CHOICE, MainWindow::on_topic_selected)
            EVT_BUTTON(ID_LOAD_DATA, MainWindow::on_load_data)
                EVT_BUTTON(ID_VALIDATE, MainWindow::on_validate)
                    EVT_BUTTON(ID_UPLOAD, MainWindow::on_upload)
                        wxEND_EVENT_TABLE()

                            MainWindow::MainWindow(const wxString &title,
                                                   AppController *controller)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)),
      m_controller(controller) {

  // Menu Bar
  auto *menu_file = new wxMenu;
  menu_file->Append(wxID_EXIT);

  auto *menu_manage = new wxMenu;
  menu_manage->Append(ID_MANAGE_PLUGINS, "&Plugins...\tCtrl-P");

  auto *menu_help = new wxMenu;
  menu_help->Append(wxID_ABOUT);

  auto *menu_bar = new wxMenuBar;
  menu_bar->Append(menu_file, "&File");
  menu_bar->Append(menu_manage, "&Manage");
  menu_bar->Append(menu_help, "&Help");
  SetMenuBar(menu_bar);

  // Main Sizer
  auto *main_sizer = new wxBoxSizer(wxVERTICAL);

  // Top Bar (Topic selection and Load)
  auto *top_sizer = new wxBoxSizer(wxHORIZONTAL);
  top_sizer->Add(new wxStaticText(this, wxID_ANY, "Topic:"), 0,
                 wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_topic_choice = new wxComboBox(this, ID_TOPIC_CHOICE, "", wxDefaultPosition,
                                  wxDefaultSize, 0, nullptr, wxCB_READONLY);
  for (const auto &topic : m_controller->get_topics()) {
    m_topic_choice->Append(wxString::FromUTF8(topic));
  }
  top_sizer->Add(m_topic_choice, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  top_sizer->Add(new wxStaticText(this, wxID_ANY, "Interface:"), 0,
                 wxALL | wxALIGN_CENTER_VERTICAL, 5);
  m_interface_choice = new wxChoice(this, ID_INTERFACE_CHOICE);
  top_sizer->Add(m_interface_choice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_load_btn = new wxButton(this, ID_LOAD_DATA, "Load...");
  m_load_btn->Enable(false);
  top_sizer->Add(m_load_btn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  main_sizer->Add(top_sizer, 0, wxEXPAND);

  // Data Preview
  main_sizer->Add(
      new wxStaticText(this, wxID_ANY, "Data Preview (First 100 records):"), 0,
      wxLEFT | wxTOP, 5);
  m_data_list = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                               wxLC_REPORT | wxBORDER_SUNKEN);
  main_sizer->Add(m_data_list, 1, wxEXPAND | wxALL, 5);

  // Control Buttons
  auto *btn_sizer = new wxBoxSizer(wxHORIZONTAL);
  m_validate_btn = new wxButton(this, ID_VALIDATE, "Validate");
  m_validate_btn->Enable(false);
  btn_sizer->Add(m_validate_btn, 0, wxALL, 5);

  m_upload_btn = new wxButton(this, ID_UPLOAD, "Upload");
  m_upload_btn->Enable(false);
  btn_sizer->Add(m_upload_btn, 0, wxALL, 5);

  main_sizer->Add(btn_sizer, 0, wxALIGN_CENTER);

  // Progress
  m_progress_gauge = new wxGauge(this, wxID_ANY, 100);
  main_sizer->Add(m_progress_gauge, 0, wxEXPAND | wxALL, 5);

  // Log Area
  main_sizer->Add(new wxStaticText(this, wxID_ANY, "Log/Status:"), 0,
                  wxLEFT | wxTOP, 5);
  m_log_ctrl =
      new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(-1, 150),
                     wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
  main_sizer->Add(m_log_ctrl, 0, wxEXPAND | wxALL, 5);

  SetSizer(main_sizer);
  CreateStatusBar();
  SetStatusText("Ready");
}

void MainWindow::update_log(const wxString &message) {
  m_log_ctrl->AppendText(message + "\n");
}

void MainWindow::update_progress(int progress) {
  m_progress_gauge->SetValue(progress);
}

void MainWindow::update_preview() {
  m_data_list->ClearAll();
  const auto &data = m_controller->get_preview_data();
  if (data.empty())
    return;

  // Create columns based on first record
  const auto &first = data[0];
  int col = 0;
  for (auto it = first.begin(); it != first.end(); ++it) {
    m_data_list->InsertColumn(col++, wxString::FromUTF8(it.key()));
  }

  // Insert data
  for (size_t i = 0; i < data.size(); ++i) {
    const auto &record = data[i];
    long index = m_data_list->InsertItem(i, "");
    int c = 0;
    for (auto it = record.begin(); it != record.end(); ++it) {
      std::string val;
      if (it.value().is_string())
        val = it.value().get<std::string>();
      else
        val = it.value().dump();
      m_data_list->SetItem(index, c++, wxString::FromUTF8(val));
    }
  }

  // Auto-size columns
  for (int i = 0; i < col; ++i) {
    m_data_list->SetColumnWidth(i, wxLIST_AUTOSIZE_USEHEADER);
  }
}

void MainWindow::clear_preview() { m_data_list->ClearAll(); }

void MainWindow::update_buttons(bool can_validate, bool can_upload) {
  m_validate_btn->Enable(can_validate);
  m_upload_btn->Enable(can_upload);
}

void MainWindow::on_exit(wxCommandEvent &WXUNUSED(event)) { Close(true); }

void MainWindow::on_about(wxCommandEvent &WXUNUSED(event)) {
  wxString about_msg = wxString::Format(
      "C GUI\n%s\n\nVersion: %s\nAuthor: %s\nLicense: %s",
      rz::config::PROJECT_DESCRIPTION,
      rz::config::VERSION,
      rz::config::AUTHOR,
      rz::config::LICENSE);

  try {
    auto result = ghupdate::check_github_update(
        std::string(rz::config::PROJECT_HOMEPAGE_URL),
        std::string(rz::config::VERSION));

    if (result.hasUpdate) {
      about_msg += wxString::Format("\n\nUpdate available: %s", result.latestVersion);
    } else {
      about_msg += "\n\nYou are running the latest version.";
    }
  } catch (const std::exception &e) {
    about_msg += wxString::Format("\n\n(Update check failed: %s)", e.what());
  }

  wxMessageBox(about_msg, "About C GUI", wxOK | wxICON_INFORMATION);
}

void MainWindow::on_manage_plugins(wxCommandEvent &WXUNUSED(event)) {
  auto plugins = m_controller->get_all_plugins();

  wxDialog dialog(this, wxID_ANY, "Manage Plugins", wxDefaultPosition, wxSize(600, 400), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
  auto *sizer = new wxBoxSizer(wxVERTICAL);

  auto *list = new wxListCtrl(&dialog, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxBORDER_SUNKEN);
  list->InsertColumn(0, "Name", wxLIST_FORMAT_LEFT, 150);
  list->InsertColumn(1, "Topic", wxLIST_FORMAT_LEFT, 80);
  list->InsertColumn(2, "Type", wxLIST_FORMAT_LEFT, 80);
  list->InsertColumn(3, "Interface", wxLIST_FORMAT_LEFT, 80);
  list->InsertColumn(4, "Version", wxLIST_FORMAT_LEFT, 80);

  for (size_t i = 0; i < plugins.size(); ++i) {
    const auto &p = plugins[i];
    long index = list->InsertItem(i, wxString::FromUTF8(p.name));
    list->SetItem(index, 1, wxString::FromUTF8(p.topic));
    list->SetItem(index, 2, wxString::FromUTF8(p.type));
    list->SetItem(index, 3, wxString::FromUTF8(p.interface_type));
    list->SetItem(index, 4, wxString::FromUTF8(p.version));
  }

  sizer->Add(list, 1, wxEXPAND | wxALL, 10);
  sizer->Add(dialog.CreateButtonSizer(wxOK), 0, wxALIGN_CENTER | wxBOTTOM, 10);

  dialog.SetSizer(sizer);
  dialog.ShowModal();
}

void MainWindow::on_topic_selected(wxCommandEvent &event) {
  std::string topic = event.GetString().ToStdString();
  m_controller->select_topic(topic);

  // Clear UI state for new topic
  m_log_ctrl->Clear();
  clear_preview();
  update_buttons(false, false);

  m_interface_choice->Clear();
  auto interfaces = m_controller->get_available_interfaces(topic);
  for (const auto &iface : interfaces) {
    m_interface_choice->Append(wxString::FromUTF8(iface));
  }

  if (!interfaces.empty()) {
    m_interface_choice->SetSelection(0);
    m_load_btn->Enable(true);
  } else {
    m_load_btn->Enable(false);
  }
}

void MainWindow::on_load_data(wxCommandEvent &WXUNUSED(event)) {
  int sel = m_interface_choice->GetSelection();
  if (sel == wxNOT_FOUND)
    return;

  std::string interface_name =
      m_interface_choice->GetString(sel).ToStdString();
  std::string path_or_conn;

  std::string current_topic = m_topic_choice->GetString(m_topic_choice->GetSelection()).ToStdString();
  std::string default_val = m_controller->get_default_data_source(current_topic, interface_name);

  // Check if we should restrict to default data source
  bool only_default = false;
  const auto& config = m_controller->get_config();
  if (config.contains("gui") && config["gui"].contains("only_default_datasource")) {
      std::string val = config["gui"]["only_default_datasource"].get<std::string>();
      only_default = (val == "true" || val == "1" || val == "yes");
  }

  if (only_default) {
      if (default_val.empty()) {
          wxMessageBox("No default data source configured for this interface.", "Error", wxOK | wxICON_ERROR);
          return;
      }
      path_or_conn = default_val;
  } else {
      if (interface_name == "csv") {
        wxFileDialog openFileDialog(this, _("Open CSV file"), "", "",
                                    "CSV files (*.csv)|*.csv",
                                    wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (!default_val.empty()) {
            openFileDialog.SetPath(wxString::FromUTF8(default_val));
        }
        if (openFileDialog.ShowModal() == wxID_CANCEL)
          return;
        path_or_conn = openFileDialog.GetPath().ToStdString();
      } else {
        wxTextEntryDialog dialog(this, "Enter connection string or path:",
                                 "Load Data (" + interface_name + ")", wxString::FromUTF8(default_val));
        if (dialog.ShowModal() == wxID_CANCEL)
          return;
        path_or_conn = dialog.GetValue().ToStdString();
      }
  }

  auto res = m_controller->load_data(interface_name, path_or_conn);
  if (!res) {
    wxMessageBox(res.error(), "Error loading Data", wxOK | wxICON_ERROR);
  }
}

void MainWindow::on_validate(wxCommandEvent &WXUNUSED(event)) {
  m_controller->start_validation();
}

void MainWindow::on_upload(wxCommandEvent &WXUNUSED(event)) {
  m_controller->start_upload();
}

} // namespace cgui
