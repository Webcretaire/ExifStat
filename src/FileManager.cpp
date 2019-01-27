#include "FileManager.h"

#include <sys/stat.h>
#include <cstring>
#include <iostream>
#include <functional>

using namespace std;

FileManager::FileManager(int allowedDepth) : allowedDepth(allowedDepth) {}

bool FileManager::ext_match(const string name, const string ext) {
    if (name.length() < ext.length())
        return false;

    for (int i = 1; i <= ext.length(); ++i)
        if (tolower(name.at(name.length() - i)) != tolower(ext.at(ext.length() - i)))
            return false;

    return true;
}

void FileManager::process_entry_dir(dirent *entry, string dir, int depth) {
    if (!string(entry->d_name).empty()) {
        bool is_dir;
#pragma omp critical
        {
            struct stat statbuf{};
            lstat((dir + "/" + entry->d_name).c_str(), &statbuf);
            is_dir = S_ISDIR(statbuf.st_mode);
        }
        if (is_dir) {
            // Found a directory, but ignore . and ..
            // Recurse at a new level if maximum depth hasn't been reached yet
            if (depth < allowedDepth
                && strcmp(".", entry->d_name) != 0
                && strcmp("..", entry->d_name) != 0)
                scandir_dir(dir + "/" + entry->d_name, depth + 1);
        }
    }
}

void FileManager::process_entry_files(dirent *entry, string dir) {
    if (!string(entry->d_name).empty()) {
        bool is_dir;
#pragma omp critical
        {
            struct stat statbuf{};
            lstat((dir + "/" + entry->d_name).c_str(), &statbuf);
            is_dir = S_ISDIR(statbuf.st_mode);
        }
        if (!is_dir) {
            // Found a file
            for (const string &ext: allowedExtensions) {
                if (ext_match(string(entry->d_name), ext)) {
                    // The file has a valid extension
#pragma omp critical
                    files.emplace_back(entry->d_name);
                    break;
                }
            }
        }
    }
}

void FileManager::scandir_dir(string dir, int depth) {
    dirs.push_back(dir);

    scandir(dir, depth, [&](dirent *d, string s, int i) {
        process_entry_dir(d, s, i);
    });
}

void FileManager::scandir_files(string dir, int depth) {
    files.clear();

    scandir(dir, depth, [&](dirent *d, string s, int i) {
        process_entry_files(d, s);
    });
}

void FileManager::scandir(string dir, int depth, function<void(dirent *, string, int)> process) {
    DIR *dp;
    const char *c_dir = dir.c_str();
    struct dirent *entry;
    if ((dp = opendir(c_dir)) == nullptr) {
        cerr << "Cannot open directory " << dir << endl;
        return;
    }
#pragma omp parallel
#pragma omp single
    {
        while ((entry = readdir(dp)) != nullptr) {
#pragma omp task
            process(entry, dir, depth);
        }
    }
    closedir(dp);
}

void FileManager::add_allowed_extension(string ext) {
    allowedExtensions.emplace_back(ext);
}

vector<string> FileManager::get_dirs() {
    return dirs;
}

vector<string> FileManager::get_files() {
    return files;
}
