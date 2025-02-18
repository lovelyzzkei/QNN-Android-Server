//
// Created by user on 2025-02-06.
//

#ifndef APP_TASKTYPE_HPP
#define APP_TASKTYPE_HPP

enum class TaskType {
    OBJECT_DETECTION = 0,
    DEPTH_ESTIMATION,
    IMAGE_CLASSIFICATION,
    SUPER_RESOLUTION
    // ...
};

namespace std {
    template <>
    struct hash<TaskType> {
        std::size_t operator()(const TaskType& t) const noexcept {
            return std::hash<int>()(static_cast<int>(t));
        }
    };
}

#endif //APP_TASKTYPE_HPP
