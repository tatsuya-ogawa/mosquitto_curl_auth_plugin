#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <shared_mutex>
class TopicManager{
private:
    struct CacheEntry {
        time_t timestamp;
        std::vector<std::string> topic_patterns;
    };
    std::unordered_map<std::string, CacheEntry> cache;
    std::shared_mutex cache_mutex;
    void add_topic_to_cache(const std::string &key, const CacheEntry &topic);
public:
    void cache_topic(std::string& buffer);
    bool is_topic_in_cache(const std::string &key, const std::string &topic);
    bool add_topic_to_cache(const std::string &key, const std::string &json_string);
};