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
  ID_MANAGE_PLUGINS,
  ID_SET_LOG_SIGNING_KEY,
  ID_SET_PROXY,
  ID_CONFIG_HELP,
  ID_DATE_FORMAT,
  ID_DATE_DELIMITER
};

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_MENU(wxID_EXIT, MainWindow::on_exit) 
    EVT_MENU(wxID_ABOUT, MainWindow::on_about)
    EVT_MENU(ID_CONFIG_HELP, MainWindow::on_config_help)
    EVT_MENU(ID_MANAGE_PLUGINS, MainWindow::on_manage_plugins)
    EVT_MENU(ID_SET_LOG_SIGNING_KEY, MainWindow::on_set_log_signing_key)
    EVT_MENU(ID_SET_PROXY, MainWindow::on_set_proxy)
        EVT_COMBOBOX(ID_TOPIC_CHOICE, MainWindow::on_topic_selected)
            EVT_BUTTON(ID_LOAD_DATA, MainWindow::on_load_data)
                EVT_BUTTON(ID_VALIDATE, MainWindow::on_validate)
                    EVT_BUTTON(ID_UPLOAD, MainWindow::on_upload)
                        EVT_CHOICE(ID_DATE_FORMAT, MainWindow::on_date_format_changed)
                            EVT_CHOICE(ID_DATE_DELIMITER, MainWindow::on_date_format_changed)
                                wxEND_EVENT_TABLE()

                            MainWindow::MainWindow(const wxString &title,
                                                   AppController *controller)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)),
      m_controller(controller) {

  // Menu Bar
  auto *menu_file = new wxMenu;
  menu_file->Append(wxID_EXIT);

  auto *menu_config = new wxMenu;
  menu_config->Append(ID_SET_LOG_SIGNING_KEY, "Set Log Signing Key...");
  menu_config->Append(ID_SET_PROXY, "Set Proxy...");

  auto *menu_manage = new wxMenu;
  menu_manage->Append(ID_MANAGE_PLUGINS, "&Plugins...\tCtrl-P");

  auto *menu_help = new wxMenu;
  menu_help->Append(ID_CONFIG_HELP, "Config Help");
  menu_help->Append(wxID_ABOUT);

  auto *menu_bar = new wxMenuBar;
  menu_bar->Append(menu_file, "&File");
  menu_bar->Append(menu_config, "&Config");
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

  // Date Format Bar
  auto *date_sizer = new wxBoxSizer(wxHORIZONTAL);
  date_sizer->Add(new wxStaticText(this, wxID_ANY, "Date Format:"), 0,
                  wxALL | wxALIGN_CENTER_VERTICAL, 5);

  m_date_format_choice = new wxChoice(this, ID_DATE_FORMAT);
  m_date_format_choice->Append("dd.mm.yyyy");
  m_date_format_choice->Append("yyyy.mm.dd");
  m_date_format_choice->Append("mm.dd.yyyy");
  m_date_format_choice->SetSelection(0);
  date_sizer->Add(m_date_format_choice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  date_sizer->Add(new wxStaticText(this, wxID_ANY, "Delimiter:"), 0,
                  wxALL | wxALIGN_CENTER_VERTICAL, 5);
  m_date_delimiter_choice = new wxChoice(this, ID_DATE_DELIMITER);
  m_date_delimiter_choice->Append(".");
  m_date_delimiter_choice->Append("/");
  m_date_delimiter_choice->Append("-");
  m_date_delimiter_choice->SetSelection(0);
  date_sizer->Add(m_date_delimiter_choice, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

  main_sizer->Add(date_sizer, 0, wxEXPAND);

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

  // Use the new async feature from gh-update-checker v1.1.0
  std::string repo_url(rz::config::PROJECT_HOMEPAGE_URL);
  
  // Sanitize URL: gh-update-checker v1.1.0 regex is strict: https://github.com/owner/repo
  // It fails on www.github.com or missing https://
  if (repo_url.find("www.github.com") != std::string::npos) {
      size_t pos = repo_url.find("www.");
      repo_url.erase(pos, 4);
  }
  if (repo_url.find("http://") == 0) {
      repo_url.replace(0, 7, "https://");
  } else if (repo_url.find("https://") != 0) {
      repo_url = "https://github.com/" + repo_url; // Assume owner/repo if prefix is missing
  }

  std::string local_version(rz::config::VERSION);

  std::string proxy;
  const auto &config = m_controller->get_config();
  if (config.contains("networking") && config["networking"].contains("proxy")) {
    proxy = config["networking"]["proxy"].get<std::string>();
  }

  // We run this in a separate thread to keep the UI responsive
  std::thread([this, about_msg, repo_url, local_version, proxy]() {
    try {
      auto future = proxy.empty() ? 
                    ghupdate::check_github_update_async(repo_url, local_version) : 
                    ghupdate::check_github_update_async(repo_url, local_version, proxy);
      auto result = future.get();

      this->CallAfter([this, about_msg, result]() {
        wxString final_msg = about_msg;
        if (result.hasUpdate) {
          final_msg += wxString::Format("\n\nUpdate available: %s", result.latestVersion);
        } else {
          final_msg += "\n\nYou are running the latest version.";
        }
        wxMessageBox(final_msg, "About C GUI", wxOK | wxICON_INFORMATION);
      });
    } catch (const std::exception &e) {
      this->CallAfter([this, about_msg, error_msg = std::string(e.what())]() {
        wxString final_msg = about_msg + wxString::Format("\n\n(Update check failed: %s)", error_msg);
        wxMessageBox(final_msg, "About C GUI", wxOK | wxICON_INFORMATION);
      });
    }
  }).detach();
}

void MainWindow::on_config_help(wxCommandEvent &WXUNUSED(event)) {
    wxString help_msg = 
        "Configuration Help:\n\n"
        "1. Log Signing Key\n"
        "Provides cryptographic integrity for your audit logs using Ed25519.\n"
        "- Activation: Select 'Config -> Set Log Signing Key'.\n"
        "- Generation: Click 'Generate Random Key' to create a unique 128-character hex key.\n"
        "- Applying: The key is active immediately and saved to your encrypted config.\n\n"
        "2. Network Proxy\n"
        "Configures a proxy server for all outgoing network connections.\n"
        "- Configuration: Select 'Config -> Set Proxy'.\n"
        "- Format: 'username:password@proxy.server.com:8080'.\n"
        "- Deactivation: Leave the field empty to disable the proxy.\n\n"
        "Note: All changes require your configuration password to re-encrypt the settings.";

    wxMessageBox(help_msg, "Config Help", wxOK | wxICON_INFORMATION);
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

  // Synchronize date format UI
  std::string effective_format = m_controller->get_effective_date_format(topic);
  if (!effective_format.empty()) {
      // Simple parser: check for common delimiters
      std::string delimiter = ".";
      if (effective_format.find('/') != std::string::npos) delimiter = "/";
      else if (effective_format.find('-') != std::string::npos) delimiter = "-";

      // Reconstruct base format (with dots for choice selection)
      std::string base_format = effective_format;
      std::replace(base_format.begin(), base_format.end(), delimiter[0], '.');

      int format_idx = m_date_format_choice->FindString(wxString::FromUTF8(base_format));
      if (format_idx != wxNOT_FOUND) {
          m_date_format_choice->SetSelection(format_idx);
      }

      int delim_idx = m_date_delimiter_choice->FindString(wxString::FromUTF8(delimiter));
      if (delim_idx != wxNOT_FOUND) {
          m_date_delimiter_choice->SetSelection(delim_idx);
      }
      
      // Update controller with current effective format
      m_controller->set_date_format(effective_format);
  }

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
      if (interface_name == "csv" || interface_name == "json") {
        wxString title = (interface_name == "csv") ? _("Open CSV file") : _("Open JSON file");
        wxString filter = (interface_name == "csv") ? "CSV files (*.csv)|*.csv" : "JSON files (*.json)|*.json";
        wxFileDialog openFileDialog(this, title, "", "",
                                    filter,
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

void MainWindow::on_date_format_changed(wxCommandEvent &WXUNUSED(event)) {
    wxString base_format = m_date_format_choice->GetString(m_date_format_choice->GetSelection());
    wxString delimiter = m_date_delimiter_choice->GetString(m_date_delimiter_choice->GetSelection());
    
    // Replace dots with selected delimiter
    wxString final_format = base_format;
    final_format.Replace(".", delimiter);
    
    m_controller->set_date_format(final_format.ToStdString());
}

void MainWindow::on_set_log_signing_key(wxCommandEvent &WXUNUSED(event)) {
    wxDialog dialog(this, wxID_ANY, "Set Log Signing Key", wxDefaultPosition, wxSize(400, 250));
    auto *sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(new wxStaticText(&dialog, wxID_ANY, "Log Signing Key (Hex):"), 0, wxALL, 10);
    
    auto *key_ctrl = new wxTextCtrl(&dialog, wxID_ANY, "");
    const auto& config = m_controller->get_config();
    if (config.contains("security") && config["security"].contains("log_signing_key")) {
        key_ctrl->SetValue(wxString::FromUTF8(config["security"]["log_signing_key"].get<std::string>()));
    }
    sizer->Add(key_ctrl, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    auto *gen_btn = new wxButton(&dialog, wxID_ANY, "Generate Random Key");
    gen_btn->Bind(wxEVT_BUTTON, [this, key_ctrl](wxCommandEvent&) {
        key_ctrl->SetValue(wxString::FromUTF8(m_controller->generate_ed25519_key()));
    });
    sizer->Add(gen_btn, 0, wxALL | wxALIGN_RIGHT, 10);

    sizer->Add(new wxStaticText(&dialog, wxID_ANY, "Config Password (to re-encrypt):"), 0, wxALL, 10);
    auto *pwd_ctrl = new wxTextCtrl(&dialog, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    sizer->Add(pwd_ctrl, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    auto *btn_sizer = dialog.CreateButtonSizer(wxOK | wxCANCEL);
    sizer->Add(btn_sizer, 0, wxALIGN_CENTER | wxALL, 10);

    dialog.SetSizer(sizer);

    if (dialog.ShowModal() == wxID_OK) {
        std::string key = key_ctrl->GetValue().ToStdString();
        std::string pwd = pwd_ctrl->GetValue().ToUTF8().data();

        if (key.empty()) {
            wxMessageBox("Key cannot be empty.", "Error", wxOK | wxICON_ERROR);
            return;
        }
        if (pwd.empty()) {
            wxMessageBox("Password is required to save changes.", "Error", wxOK | wxICON_ERROR);
            return;
        }

        auto res = m_controller->update_log_signing_key(key, pwd);
        if (!res) {
            wxMessageBox(wxString::Format("Failed to update key: %s", res.error().c_str()), "Error", wxOK | wxICON_ERROR);
        } else {
            wxMessageBox("Log signing key updated successfully.", "Success", wxOK | wxICON_INFORMATION);
        }
    }
}

void MainWindow::on_set_proxy(wxCommandEvent &WXUNUSED(event)) {
    wxDialog dialog(this, wxID_ANY, "Set Network Proxy", wxDefaultPosition, wxSize(400, 250));
    auto *sizer = new wxBoxSizer(wxVERTICAL);

    sizer->Add(new wxStaticText(&dialog, wxID_ANY, "Proxy (user:pass@host:port):"), 0, wxALL, 10);
    
    auto *proxy_ctrl = new wxTextCtrl(&dialog, wxID_ANY, "");
    const auto& config = m_controller->get_config();
    std::string current_proxy;
    if (config.contains("networking") && config["networking"].contains("proxy")) {
        current_proxy = config["networking"]["proxy"].get<std::string>();
        proxy_ctrl->SetValue(wxString::FromUTF8(current_proxy));
    }
    
    if (current_proxy.empty()) {
        proxy_ctrl->SetHint("myuser:mypass@proxy.example.com:8080");
    }
    
    sizer->Add(proxy_ctrl, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    sizer->Add(new wxStaticText(&dialog, wxID_ANY, "Config Password (to re-encrypt):"), 0, wxALL, 10);
    auto *pwd_ctrl = new wxTextCtrl(&dialog, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
    sizer->Add(pwd_ctrl, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

    auto *btn_sizer = dialog.CreateButtonSizer(wxOK | wxCANCEL);
    sizer->Add(btn_sizer, 0, wxALIGN_CENTER | wxALL, 10);

    dialog.SetSizer(sizer);

    if (dialog.ShowModal() == wxID_OK) {
        std::string proxy = proxy_ctrl->GetValue().ToStdString();
        std::string pwd = pwd_ctrl->GetValue().ToStdString();

        if (pwd.empty()) {
            wxMessageBox("Password is required to save changes.", "Error", wxOK | wxICON_ERROR);
            return;
        }

        auto res = m_controller->update_proxy(proxy, pwd);
        if (!res) {
            wxMessageBox(wxString::Format("Failed to update proxy: %s", res.error().c_str()), "Error", wxOK | wxICON_ERROR);
        } else {
            wxMessageBox("Proxy setting updated successfully.", "Success", wxOK | wxICON_INFORMATION);
        }
    }
}

} // namespace cgui
