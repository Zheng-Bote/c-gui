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
#include <wx/filedlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace cgui {

enum {
  ID_TOPIC_CHOICE = wxID_HIGHEST + 1,
  ID_LOAD_CSV,
  ID_VALIDATE,
  ID_UPLOAD
};

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(wxID_EXIT, MainWindow::on_exit) EVT_MENU(wxID_ABOUT,
                                                      MainWindow::on_about)
        EVT_COMBOBOX(ID_TOPIC_CHOICE, MainWindow::on_topic_selected)
            EVT_BUTTON(ID_LOAD_CSV, MainWindow::on_load_csv)
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
  auto *menu_help = new wxMenu;
  menu_help->Append(wxID_ABOUT);
  auto *menu_bar = new wxMenuBar;
  menu_bar->Append(menu_file, "&File");
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

  m_load_btn = new wxButton(this, ID_LOAD_CSV, "Load CSV...");
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
  wxMessageBox(
      "C GUI\nA C++23 wxWidgets Application for Data Validation and Upload.",
      "About C GUI", wxOK | wxICON_INFORMATION);
}

void MainWindow::on_topic_selected(wxCommandEvent &event) {
  m_controller->select_topic(event.GetString().ToStdString());
}

void MainWindow::on_load_csv(wxCommandEvent &WXUNUSED(event)) {
  wxFileDialog openFileDialog(this, _("Open CSV file"), "", "",
                              "CSV files (*.csv)|*.csv",
                              wxFD_OPEN | wxFD_FILE_MUST_EXIST);
  if (openFileDialog.ShowModal() == wxID_CANCEL)
    return;

  auto res = m_controller->load_csv(openFileDialog.GetPath().ToStdString());
  if (!res) {
    wxMessageBox(res.error(), "Error loading CSV", wxOK | wxICON_ERROR);
  }
}

void MainWindow::on_validate(wxCommandEvent &WXUNUSED(event)) {
  m_controller->start_validation();
}

void MainWindow::on_upload(wxCommandEvent &WXUNUSED(event)) {
  m_controller->start_upload();
}

} // namespace cgui
