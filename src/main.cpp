/**
 * SPDX-FileComment: Main entry point for the c-gui application
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file src/main.cpp
 * @brief Application entry point and initialization.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#include "AppController.hpp"
#include "MainWindow.hpp"
#include <memory>
#include <wx/wx.h>

/**
 * @class CGuiApp
 * @brief Main application class for wxWidgets.
 */
class CGuiApp : public wxApp {
public:
  /**
   * @brief Initializes the application.
   * @return true if successful, false otherwise.
   */
  virtual bool OnInit() override {
    if (!wxApp::OnInit())
      return false;

    m_controller = std::make_unique<cgui::AppController>();

    // Ask for password
    wxPasswordEntryDialog dialog(
        nullptr,
        "Enter password to decrypt configuration:", "Configuration Password");
    if (dialog.ShowModal() != wxID_OK) {
      return false;
    }

    std::string password = dialog.GetValue().ToStdString();
    std::filesystem::path ini_path =
        "config.enc"; // Default path, could be configurable

    if (!std::filesystem::exists(ini_path)) {
      wxMessageBox("Configuration file 'config.enc' not found.", "Error",
                   wxOK | wxICON_ERROR);
      return false;
    }

    auto init_res = m_controller->init(ini_path, password);
    if (!init_res) {
      wxMessageBox(wxString::Format("Failed to initialize: %s",
                                    init_res.error().c_str()),
                   "Error", wxOK | wxICON_ERROR);
      return false;
    }

    auto *frame = new cgui::MainWindow("C GUI", m_controller.get());
    m_controller->set_window(frame);
    frame->Show(true);

    return true;
  }

private:
  std::unique_ptr<cgui::AppController> m_controller;
};

wxIMPLEMENT_APP(CGuiApp);
