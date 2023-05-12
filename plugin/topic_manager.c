#include <nlohmann/json.hpp>
#include <mutex>
#include "topic_manager.h"
using json = nlohmann::json;


bool is_topic_valid(const char *pattern, const char *topic) {
    while (*pattern) {
        if (*pattern == '+') {
            pattern++;
            while (*topic && *topic != '/') {
                topic++;
            }
            if (*topic == '\0' && *pattern != '\0') {
                return false;
            }
        } else if (*pattern == '#') {
            pattern++;
            while (*topic) {
                topic++;
            }
            return *pattern == '\0';
        } else {
            if (*pattern != *topic) {
                return false;
            }
            pattern++;
            topic++;
        }
    }
    return *topic == '\0';
}

void TopicManager::cache_topic(std::string& buffer){
    json json = json::parse(buffer);
}

void TopicManager::add_topic_to_cache(const std::string &key, const CacheEntry &entry) {
    std::unique_lock<std::shared_mutex> lock(cache_mutex);
    cache[key] = std::move(entry);
}
bool TopicManager::add_topic_to_cache(const std::string &key, const std::string &json_string) {
    try {
        auto json = json::parse(json_string);
        if (json.contains("topic_patterns") && json["topic_patterns"].is_array()) {
            CacheEntry entry;
            entry.topic_patterns = json["topic_patterns"].get<std::vector<std::string>>();
            entry.timestamp = std::time(nullptr);
            cache[key] = entry;
            return true;
        }
        return false;
    } catch (const nlohmann::json::exception &e) {
        return false;
    }
}
bool TopicManager::is_topic_in_cache(const std::string &key, const std::string &topic) {
    std::shared_lock<std::shared_mutex> lock(cache_mutex);
    auto it = cache.find(key);
    if (it != cache.end()){
        return std::any_of(it->second.topic_patterns.begin(),it->second.topic_patterns.end(),[&topic](std::string& pattern){
            return is_topic_valid(pattern.c_str(),topic.c_str());
        });
    }
    return false;
}
