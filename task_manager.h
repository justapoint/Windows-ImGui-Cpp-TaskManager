#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include "task.h"
#include "json.hpp"

// Windows API includes for UUID generation
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>

using json = nlohmann::json;

// Main class for managing tasks with persistence and selection functionality
class TaskManager {
private:
    // Storage for tasks using UUID as key for efficient lookup
    std::unordered_map<std::string, Task> tasks;
    
    // Track which tasks are selected in the UI (for bulk operations)
    std::unordered_map<std::string, bool> task_selection_states;
    
    // File path for JSON data persistence
    std::string filename;
    
    // Flag indicating if COM is initialized for UUID generation
    bool com_initialized = false;
    
    // Generate unique identifier for new tasks
    std::string generate_uuid() const;

public:
    // Constructor: Initializes task manager with data file path
    TaskManager(const std::string& data_file = "data.json");
    
    // Destructor: Saves data and cleans up resources
    ~TaskManager();

    // Save all tasks to JSON file
    void save();
    
    // Load tasks from JSON file
    void load();

    // Core task operations
    
    // Add a new task with given title (automatically generates UUID)
    void add_task(const std::string& title);
    
    // Delete task by its unique identifier
    void delete_task(const std::string& task_id);
    
    // Toggle completion status of a task (complete/incomplete)
    void toggle_task_status(const std::string& task_id);

    // Task query methods
    
    // Check if a task exists with the given ID
    bool contains_task(const std::string& task_id) const;
    
    // Check if there are no tasks in the manager
    bool is_empty() const;
    
    // Get all tasks as a vector (for iteration)
    std::vector<Task> get_all_tasks() const;
    
    // Get only uncompleted (active) tasks
    std::vector<Task> get_uncompleted_tasks() const;
    
    // Get only completed tasks
    std::vector<Task> get_completed_tasks() const;
    
    // Get specific task information by ID
    Task get_task_info(const std::string& task_id) const;

    // Task selection methods (for UI bulk operations)
    
    // Initialize selection states for all tasks (call before showing selection modal)
    void init_selection_states();
    
    // Toggle selection state for a specific task
    void toggle_task_selection(const std::string& task_id);
    
    // Check if a task is currently selected
    bool is_task_selected(const std::string& task_id) const;
    
    // Clear all task selections
    void clear_selection();
    
    // Get list of IDs for all selected tasks
    std::vector<std::string> get_selected_task_ids() const;
    
    // Check if any tasks are currently selected
    bool has_selection() const;
};

#endif