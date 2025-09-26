#include <iostream>
#include "task_manager.h"
#include <fstream>
#include <filesystem>

// Constructor: Initializes TaskManager with data file path
TaskManager::TaskManager(const std::string& data_file) : filename(data_file) {
    // Initialize COM for UUID generation (required for CoCreateGuid)
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    com_initialized = SUCCEEDED(hr);
    
    if (!com_initialized) {
        std::cerr << "Warning: COM initialization failed. UUID generation may not work." << std::endl;
    }

    load();  // Load tasks from file on initialization
}

// Destructor: Saves tasks and cleans up COM
TaskManager::~TaskManager() {
    save();  // Save tasks to file before destruction

    if (com_initialized) {
        CoUninitialize();  // Clean up COM
    }
}

// Load tasks from JSON file
void TaskManager::load() {
    tasks.clear();  // Clear existing tasks

    // Create data file if it doesn't exist
    if (!std::filesystem::exists(filename)) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot create data file " << filename << std::endl;
            return;
        }
        file << "{}";  // Write empty JSON object
        return;
    }

    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot open data file " << filename << std::endl;
            return;
        }

        // Check if file is empty
        if (file.peek() == std::ifstream::traits_type::eof()) {
            std::cerr << "Warning: Data file is empty, creating new one" << std::endl;
            file.close();
            std::ofstream new_file(filename);
            new_file << "{}";  // Create valid empty JSON
            return;
        }

        json j;
        file >> j;  // Parse JSON from file

        // Validate JSON structure
        if (!j.is_object()) {
            std::cerr << "Warning: Invalid JSON structure, creating new file" << std::endl;
            std::ofstream new_file(filename);
            new_file << "{}";  // Reset to valid JSON
            return;
        }

        // Process each task in the JSON
        for (const auto& [id, task_data] : j.items()) {
            try {
                // Validate required fields exist
                if (task_data.contains("title") && task_data.contains("done")) {
                    tasks[id] = Task(id, task_data["title"], task_data["done"]);
                } else {
                    std::cerr << "Warning: Skipping task " << id << " - missing required fields" << std::endl;
                }
            } catch (const json::exception& e) {
                std::cerr << "Warning: Skipping task " << id << " - invalid data: " << e.what() << std::endl;
            }
        }

    } catch (const json::parse_error& e) {
        // Handle JSON parsing errors
        std::cerr << "Error: JSON parse error in " << filename << ": " << e.what() << std::endl;
        std::cerr << "Creating new data file..." << std::endl;
        
        std::ofstream file(filename);
        file << "{}";  // Reset file with valid JSON
        tasks.clear();  // Clear any partially loaded tasks
    } catch (const std::exception& e) {
        std::cerr << "Error loading data from " << filename << ": " << e.what() << std::endl;
    }
}

// Save tasks to JSON file
void TaskManager::save() {
    try {
        json j = json::object();  // Create root JSON object
        
        // Convert each task to JSON format
        for (const auto& [id, task] : tasks) {
            j[id] = {
                {"title", task.title},
                {"done", task.done}
            };
        }
        
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Cannot save to file " << filename << std::endl;
            return;
        }
        
        file << j.dump(4);  // Write JSON with 4-space indentation
        
    } catch (const std::exception& e) {
        std::cerr << "Error saving data to " << filename << ": " << e.what() << std::endl;
    }
}

// Generate UUID for new tasks
std::string TaskManager::generate_uuid() const {
    // Fallback if COM is not initialized
    if (!com_initialized) {
        std::cerr << "Error: COM not initialized, using fallback UUID" << std::endl;
        return "fallback-uuid-" + std::to_string(rand());
    }
    
    GUID guid;
    HRESULT hr = CoCreateGuid(&guid);  // Generate GUID using Windows API
    
    if (FAILED(hr)) {
        std::cerr << "Error: Failed to create GUID" << std::endl;
        return "error-uuid-" + std::to_string(rand());
    }
    
    // Format GUID as string (8-4-4-4-12 format)
    char buffer[37];
    snprintf(buffer, sizeof(buffer),
             "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
             guid.Data1, guid.Data2, guid.Data3,
             guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
             guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    
    return std::string(buffer);
}

// Add a new task with the given title
void TaskManager::add_task(const std::string& title) {
    std::string task_id = generate_uuid();  // Generate unique ID
    tasks[task_id] = Task(task_id, title, false);  // Create new task (not done)
    save();  // Persist changes to file
}

// Delete a task by ID
void TaskManager::delete_task(const std::string& task_id) {
    tasks.erase(task_id);  // Remove task from map
    save();  // Persist changes to file
}

// Toggle task completion status
void TaskManager::toggle_task_status(const std::string& task_id) {
    auto it = tasks.find(task_id);
    if (it != tasks.end()) {
        Task& task = it->second;
        task.done = !task.done;  // Flip completion status
        save();  // Persist changes to file
    }
}

// Check if task exists by ID
bool TaskManager::contains_task(const std::string& task_id) const {
    return tasks.find(task_id) != tasks.end();
}

// Check if there are no tasks
bool TaskManager::is_empty() const {
    return tasks.empty();
}

// Get all tasks as a vector
std::vector<Task> TaskManager::get_all_tasks() const {
    std::vector<Task> result;
    for (const auto& [id, task] : tasks) {
        result.push_back(task);  // Copy each task to vector
    }
    return result;
}

// Get only uncompleted tasks
std::vector<Task> TaskManager::get_uncompleted_tasks() const {
    std::vector<Task> result;
    for (const auto& [id, task] : tasks) {
        if (!task.done) {
            result.push_back(task);  // Add only incomplete tasks
        }
    }
    return result;
}

// Get only completed tasks
std::vector<Task> TaskManager::get_completed_tasks() const {
    std::vector<Task> result;
    for (const auto& [id, task] : tasks) {
        if (task.done) {
            result.push_back(task);  // Add only completed tasks
        }
    }
    return result;
}

// Get task information by ID
Task TaskManager::get_task_info(const std::string& task_id) const {
    auto it = tasks.find(task_id);
    if (it != tasks.end()) {
        return it->second;  // Return found task
    }
    return Task("", "");  // Return empty task if not found
}

// Initialize selection states for all tasks (used in UI modals)
void TaskManager::init_selection_states() {
    task_selection_states.clear();
    for (const auto& [id, task] : tasks) {
        task_selection_states[id] = false;  // Initialize all as unselected
    }
}

// Toggle selection state for a specific task
void TaskManager::toggle_task_selection(const std::string& task_id) {
    if (task_selection_states.find(task_id) != task_selection_states.end()) {
        task_selection_states[task_id] = !task_selection_states[task_id];  // Flip selection state
    } else {
        task_selection_states[task_id] = true;  // Select if not in map
    }
}

// Check if a task is currently selected
bool TaskManager::is_task_selected(const std::string& task_id) const {
    auto it = task_selection_states.find(task_id);
    return it != task_selection_states.end() ? it->second : false;  // Return false if not found
}

// Clear all task selections
void TaskManager::clear_selection() {
    for (auto& [id, selected] : task_selection_states) {
        selected = false;  // Deselect all tasks
    }
}

// Get IDs of all selected tasks
std::vector<std::string> TaskManager::get_selected_task_ids() const {
    std::vector<std::string> selected;
    for (const auto& [id, selected_state] : task_selection_states) {
        if (selected_state) {
            selected.push_back(id);  // Add ID if task is selected
        }
    }
    return selected;
}

// Check if any tasks are currently selected
bool TaskManager::has_selection() const {
    for (const auto& [id, selected] : task_selection_states) {
        if (selected) return true;  // Return true if any task is selected
    }
    return false;
}