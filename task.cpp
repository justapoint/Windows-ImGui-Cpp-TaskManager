#include "task.h"

// Task constructor implementation
// Initializes a task with the provided ID, title, and completion status
Task::Task(const std::string& task_id, const std::string& task_title, bool task_done)
    : id(task_id),        // Initialize task ID
      title(task_title),  // Initialize task title/description
      done(task_done)     // Initialize completion status
{}