#include <iostream>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "task_manager.h"
#include "task.h"

// Application state structure to manage UI modal states and input buffers
struct AppState {
    bool show_add_task_modal = false;           // Flag to show/hide add task modal
    bool show_mark_task_modal = false;          // Flag to show/hide mark task modal
    bool show_mark_delete_task_modal = false;   // Flag to show/hide delete task modal
    
    char task_input_buffer[256] = "";           // Buffer for storing new task input
    bool task_input_focused = false;            // Flag to focus on task input field
};

// Function to draw a modal for task selection (marking or deletion)
// Returns true if user confirms the action, false otherwise
bool DrawTaskSelectionModal(
    const char* title,              // Modal title
    const char* confirm_button_text,// Text for confirm button
    TaskManager& task_manager,      // Reference to task manager
    bool& show_modal_flag)          // Flag to control modal visibility
{
    bool result = false;            // Return value indicating user action
    
    // Open modal when flag is set
    if (show_modal_flag) {
        ImGui::OpenPopup(title);
        task_manager.init_selection_states();  // Initialize selection states
        show_modal_flag = false;               // Reset flag after opening
    }

    // Set modal position and size
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(750, 600), ImGuiCond_FirstUseEver);

    // Begin modal popup
    if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar)) {
        // Close modal on Escape key press
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            task_manager.clear_selection();  // Clear any selections
            ImGui::CloseCurrentPopup();      // Close modal
        }

        // Center the title text
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize(title).x) * 0.5f);
        ImGui::Text("%s", title);
        ImGui::Separator();
        
        // Calculate content area height (remaining space minus button area)
        float content_height = ImGui::GetContentRegionAvail().y - 50;
        ImGui::BeginChild("ContentArea", ImVec2(0, content_height), false, ImGuiWindowFlags_NoScrollbar);
        
        // Create two columns for uncompleted and completed tasks
        ImGui::Columns(2, "task_columns", true);
        float column_width = (ImGui::GetWindowWidth() - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
        ImGui::SetColumnWidth(0, column_width);  // Set left column width
        ImGui::SetColumnWidth(1, column_width);  // Set right column width

        // Left column: Uncompleted tasks
        ImGui::BeginChild("LeftColumn", ImVec2(0, 0), false);
        ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "To Complete");
        ImGui::Separator();
        
        // Scrollable area for uncompleted tasks
        ImGui::BeginChild("LeftScroll", ImVec2(0, -1), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
        std::vector<Task> uncompleted_tasks = task_manager.get_uncompleted_tasks();

        // Display each uncompleted task with checkbox
        int display_index = 1;
        for (const Task& task : uncompleted_tasks) {
            std::string task_label = std::to_string(display_index) + ". " + task.title;
            
            // Checkbox for task selection
            bool current_selection = task_manager.is_task_selected(task.id);
            if (ImGui::Checkbox(("##task_" + task.id).c_str(), &current_selection)) {
                task_manager.toggle_task_selection(task.id);  // Toggle selection state
            }
            
            ImGui::SameLine();
            ImGui::TextWrapped("%s", task_label.c_str());  // Display task text
            display_index++;
        }
        ImGui::EndChild();  // End LeftScroll
        ImGui::EndChild();  // End LeftColumn

        // Right column: Completed tasks
        ImGui::NextColumn();
        ImGui::BeginChild("RightColumn", ImVec2(0, 0), false);
        ImGui::TextColored(ImVec4(0.5f, 1, 0.5f, 1), "Completed");
        ImGui::Separator();
        
        // Scrollable area for completed tasks
        ImGui::BeginChild("RightScroll", ImVec2(0, -1), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
        std::vector<Task> completed_tasks = task_manager.get_completed_tasks();

        // Display each completed task with checkbox
        display_index = 1;
        for (const Task& task : completed_tasks) {
            std::string task_label = std::to_string(display_index) + ". " + task.title;
            
            // Checkbox for task selection
            bool current_selection = task_manager.is_task_selected(task.id);
            if (ImGui::Checkbox(("##task_" + task.id).c_str(), &current_selection)) {
                task_manager.toggle_task_selection(task.id);  // Toggle selection state
            }
            
            ImGui::SameLine();
            ImGui::TextWrapped("%s", task_label.c_str());  // Display task text
            display_index++;
        }
        ImGui::EndChild();  // End RightScroll
        ImGui::EndChild();  // End RightColumn

        ImGui::Columns(1);  // Reset to single column
        ImGui::EndChild();  // End ContentArea
        ImGui::Separator();
        
        // Button area at the bottom of the modal
        ImGui::BeginChild("ButtonArea", ImVec2(0, 0), false);
        
        // Check if any tasks are selected
        bool has_selection = task_manager.has_selection();

        // Calculate button positioning
        float button_width = 100;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float total_width = button_width * 2 + spacing;
        float start_x = (ImGui::GetWindowWidth() - total_width) * 0.5f;
        
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);  // Add vertical spacing
        
        // Disable confirm button if no tasks are selected
        if (!has_selection) {
            ImGui::BeginDisabled();
        }
        
        ImGui::SetCursorPosX(start_x);
        
        // Confirm button - performs the action (mark/delete)
        if (ImGui::Button(confirm_button_text, ImVec2(button_width, 30))) {
            result = true;  // User confirmed the action
            ImGui::CloseCurrentPopup();  // Close modal
        }  
        
        if (!has_selection) {
            ImGui::EndDisabled();
        }
        
        ImGui::SameLine();
        
        // Cancel button - closes modal without action
        if (ImGui::Button("Cancel", ImVec2(button_width, 30))) {
            task_manager.clear_selection();  // Clear selections
            ImGui::CloseCurrentPopup();      // Close modal
        }
        
        ImGui::EndChild();  // End ButtonArea
        ImGui::EndPopup();  // End modal
    }
    
    return result;  // Return whether user confirmed the action
}

// GLFW error callback function
static void glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Function to check for OpenGL errors
void check_gl_errors(const char* context) {
    GLenum error;
    // Check for all pending OpenGL errors
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL Error in " << context << ": ";
        // Print human-readable error message
        switch (error) {
            case GL_INVALID_ENUM: std::cerr << "GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE: std::cerr << "GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION: std::cerr << "GL_INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY: std::cerr << "GL_OUT_OF_MEMORY"; break;
            default: std::cerr << "0x" << std::hex << error; break;
        }
        std::cerr << std::endl;
    }
}

// Main application entry point
int main() {
    // Set GLFW error callback
    glfwSetErrorCallback(glfw_error_callback);
    
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Error: Failed to initialize GLFW\n";
        return -1;
    }

    // Create GLFW window
    GLFWwindow* window = glfwCreateWindow(1200, 800, "Todo List", nullptr, nullptr);
    if (!window) {
        std::cerr << "Error: Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);  // Make the window's context current
    glfwSwapInterval(1);  // Enable vsync

    // Check for OpenGL errors after initialization
    check_gl_errors("Initialization");

    // Initialize Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    ImGui::StyleColorsDark();  // Set dark theme

    // Initialize ImGui backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Initialize application state and task manager
    TaskManager task_manager;
    AppState state;

    // Main application loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();  // Process events

        // Start new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Get current window size
        int window_width, window_height;
        glfwGetWindowSize(window, &window_width, &window_height);

        // Create main application window (fullscreen)
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2((float)window_width, (float)window_height));
        ImGui::Begin("MainWindow", nullptr, 
            ImGuiWindowFlags_NoTitleBar | 
            ImGuiWindowFlags_NoResize | 
            ImGuiWindowFlags_NoMove);

        // Calculate panel widths with margins
        float margin = 20.0f;
        float panel_width = (window_width - 3 * margin) / 3;
        
        // Left panel: Control buttons
        ImGui::BeginChild("LeftPanel", ImVec2(panel_width, -1), true);
        
        ImGui::Text("Control Panel");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));  // Add spacing
        
        // Button to mark tasks as complete/incomplete
        if (ImGui::Button("Mark Task", ImVec2(-1, 0))) {
            state.show_mark_task_modal = true;  // Show mark task modal
        }
        ImGui::Dummy(ImVec2(0, 10));
        
        // Button to add new task
        if (ImGui::Button("Add New Task", ImVec2(-1, 0))) {
            state.show_add_task_modal = true;  // Show add task modal
            state.task_input_buffer[0] = '\0'; // Clear input buffer
            state.task_input_focused = true;   // Set focus flag
        }
        ImGui::Dummy(ImVec2(0, 10));
        
        // Button to delete tasks
        if (ImGui::Button("Delete Task", ImVec2(-1, 0))) {
            state.show_mark_delete_task_modal = true;  // Show delete task modal
        }
        ImGui::Dummy(ImVec2(0, 10));
        
        // Exit button
        if (ImGui::Button("Exit", ImVec2(-1, 0))) {
            glfwSetWindowShouldClose(window, true);  // Close application
        }
        
        ImGui::EndChild();  // End LeftPanel

        ImGui::SameLine(0, margin);  // Move to next panel with margin

        // Center panel: Uncompleted tasks list
        ImGui::BeginChild("CenterPanel", ImVec2(panel_width, -1), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

        ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "Uncompleted Tasks");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));

        // Get and display uncompleted tasks
        std::vector<Task> uncompleted_tasks = task_manager.get_uncompleted_tasks();

        if (uncompleted_tasks.empty()) {
            ImGui::Text("No uncompleted tasks information");
        } else {
            int task_index = 1;
            for (const Task& task : uncompleted_tasks) {
                std::string task_label = std::to_string(task_index) + ". " + task.title;
                ImGui::TextWrapped("%s", task_label.c_str());  // Display task with wrapping
                task_index++;
                ImGui::Dummy(ImVec2(0, 10));  // Add spacing between tasks
            }
        }

        ImGui::EndChild();  // End CenterPanel

        ImGui::SameLine(0, margin);  // Move to next panel with margin

        // Right panel: Completed tasks list
        ImGui::BeginChild("RightPanel", ImVec2(panel_width, -1), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

        ImGui::TextColored(ImVec4(0.5f, 1, 0.5f, 1), "Completed Tasks");
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 10));

        // Get and display completed tasks
        std::vector<Task> completed_tasks = task_manager.get_completed_tasks();

        if (completed_tasks.empty()) {
            ImGui::Text("No completed tasks");
        } else {
            int task_index = 1;
            for (const Task& task : completed_tasks) {
                std::string task_label = std::to_string(task_index) + ". " + task.title;
                ImGui::TextWrapped("%s", task_label.c_str());  // Display task with wrapping
                task_index++;
                ImGui::Dummy(ImVec2(0, 10));  // Add spacing between tasks
            }
        }

        ImGui::EndChild();  // End RightPanel

        // Handle mark task modal
        if (state.show_mark_task_modal) {
            ImGui::OpenPopup("Mark Tasks");
            state.show_mark_task_modal = false;
        }

        // If user confirms marking tasks, toggle their status
        if (DrawTaskSelectionModal("Mark Tasks", "Confirm", task_manager, state.show_mark_task_modal)) {
            auto selected_ids = task_manager.get_selected_task_ids();
            for (const std::string& task_id : selected_ids) {
                task_manager.toggle_task_status(task_id);  // Toggle completion status
            }
            task_manager.clear_selection();  // Clear selection after operation
        }

        // Handle delete task modal
        if (state.show_mark_delete_task_modal) {
            ImGui::OpenPopup("Delete Tasks");
            state.show_mark_delete_task_modal = false;
        }

        // If user confirms deletion, delete selected tasks
        if (DrawTaskSelectionModal("Delete Tasks", "Confirm", task_manager, state.show_mark_delete_task_modal)) {
            auto selected_ids = task_manager.get_selected_task_ids();
            for (const std::string& task_id : selected_ids) {
                task_manager.delete_task(task_id);  // Delete task
            }
            task_manager.clear_selection();  // Clear selection after operation
        }

        // Handle add task modal
        if (state.show_add_task_modal) {
            ImGui::OpenPopup("Add New Task");
            state.show_add_task_modal = false;
        }

        // Position add task modal in center of screen
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        // Add new task modal
        if (ImGui::BeginPopupModal("Add New Task", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            // Close modal on Escape key
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                ImGui::CloseCurrentPopup();
                state.task_input_buffer[0] = '\0';  // Clear input buffer
            }

            ImGui::Text("Enter task name:");
            
            // Set keyboard focus to input field if requested
            if (state.task_input_focused) {
                ImGui::SetKeyboardFocusHere();
                state.task_input_focused = false;
            }
            
            // Input text field for new task name
            if (ImGui::InputText("##task_input", state.task_input_buffer, IM_ARRAYSIZE(state.task_input_buffer), 
                                ImGuiInputTextFlags_EnterReturnsTrue)) {
                // Add task when Enter is pressed and input is not empty
                if (strlen(state.task_input_buffer) > 0) {
                    std::string task_string = state.task_input_buffer;
                    task_manager.add_task(task_string);  // Add new task
                    ImGui::CloseCurrentPopup();          // Close modal
                }
            }
            
            ImGui::Separator();
            
            // Center the buttons
            float width = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX((width - 200) * 0.5f);
            
            // Disable confirm button if input is empty
            if (strlen(state.task_input_buffer) == 0) {
                ImGui::BeginDisabled();
            }
            
            // Confirm button to add task
            if (ImGui::Button("Confirm", ImVec2(80, 0))) {
                std::string task_string = state.task_input_buffer;
                task_manager.add_task(task_string);  // Add new task
                ImGui::CloseCurrentPopup();          // Close modal
            }
            
            if (strlen(state.task_input_buffer) == 0) {
                ImGui::EndDisabled();
            }
            
            ImGui::SameLine();
            
            // Cancel button to close modal without adding task
            if (ImGui::Button("Cancel", ImVec2(80, 0))) {
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::EndPopup();  // End add task modal
        }

        ImGui::End();  // End MainWindow

        // Render ImGui and swap buffers
        ImGui::Render();
        glClearColor(0.15f, 0.15f, 0.2f, 1.0f);  // Set clear color
        glClear(GL_COLOR_BUFFER_BIT);             // Clear screen
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());  // Render ImGui
        glfwSwapBuffers(window);                  // Swap front and back buffers
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}