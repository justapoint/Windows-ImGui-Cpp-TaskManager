#ifndef TASK_H
#define TASK_H

#include <string>

// Task structure representing a single todo item
struct Task {
    std::string id;        // Unique identifier for the task
    std::string title;     // Task description or name
    bool done;             // Completion status (true = completed, false = pending)
    
    // Constructor: Creates a new task with specified parameters
    // Default values create an empty/invalid task
    Task(const std::string& task_id = "", const std::string& task_title = "", bool task_done = false);
};

#endif