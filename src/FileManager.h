#ifndef EXIFSTAT_FILEMANAGER_H
#define EXIFSTAT_FILEMANAGER_H

#include <string>
#include <dirent.h>
#include <vector>
#include <functional>

using namespace std;

class FileManager {
public:
    FileManager(int allowedDepth);

    bool ext_match(string name, string ext);

    void scandir_dir(string dir, int depth = 0);

    void scandir_files(string dir, int depth = 0);

    void add_allowed_extension(string ext);

    vector<string> get_dirs();

    vector<string> get_files();

private:
    vector<string> allowedExtensions;
    vector<string> dirs, files;
    int allowedDepth;

    void process_entry_dir(dirent *entry, string dir, int depth);

    void process_entry_files(dirent *entry, string dir);

    void scandir(string dir, int depth, function<void(dirent *, string, int)> process);

};

#endif //EXIFSTAT_FILEMANAGER_H
