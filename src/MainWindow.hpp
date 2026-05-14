/**
 * SPDX-FileComment: MainWindow for c-gui
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2025 ZHENG Robert
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file MainWindow.hpp
 * @brief Main GUI frame for the application.
 * @version 1.0.0
 * @date 2025-02-13
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2025 ZHENG Robert
 * @license Apache-2.0
 */

#ifndef C_GUI_MAIN_WINDOW_HPP
#define C_GUI_MAIN_WINDOW_HPP

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <vector>
#include <string>
#include "Validator.hpp"

namespace cgui {

class AppController;

/**
 * @class MainWindow
 * @brief Main frame of the application.
 */
class MainWindow : public wxFrame {
public:
    MainWindow(const wxString& title, AppController* controller);
    ~MainWindow() = default;

    /**
     * @brief Append a message to the log area.
     * @param message The message to log.
     */
    void update_log(const wxString& message);

    /**
     * @brief Update the progress gauge.
     * @param progress Progress value (0-100).
     */
    void update_progress(int progress);

    /**
     * @brief Update the data preview table.
     */
    void update_preview();

    /**
     * @brief Clear the data preview table.
     */
    void clear_preview();

    /**
     * @brief Enable/disable validation and upload buttons.
     * @param can_validate If validation can be started.
     * @param can_upload If upload can be started.
     */
    void update_buttons(bool can_validate, bool can_upload);

private:
    AppController* m_controller;

    wxComboBox* m_topic_choice;
    wxButton* m_load_btn;
    wxButton* m_validate_btn;
    wxButton* m_upload_btn;
    wxListCtrl* m_data_list;
    wxTextCtrl* m_log_ctrl;
    wxGauge* m_progress_gauge;

    void on_exit(wxCommandEvent& event);
    void on_about(wxCommandEvent& event);
    void on_topic_selected(wxCommandEvent& event);
    void on_load_csv(wxCommandEvent& event);
    void on_validate(wxCommandEvent& event);
    void on_upload(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
};

} // namespace cgui

#endif // C_GUI_MAIN_WINDOW_HPP
