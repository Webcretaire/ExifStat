#include <getopt.h>
#include <iostream>
#include <omp.h>
#include "FileManager.h"
#include "../cpp_exiftool/inc/ExifTool.h"
#include "../cpp_exiftool/inc/TagInfo.h"
#include <map>
#include <cmath>

using namespace std;

void display_help() {
    cout << "Usage : ./ExifStat [OPTIONS]" << endl
         << "Options : " << endl
         << "   -h" << endl
         << "       Displays this help message and exits" << endl
         << "   -f FOLDER" << endl
         << "       Specifies the folder to explore (default = .)" << endl
         << "   -e EXIFTOOL" << endl
         << "       Specifies the location of exiftool executable (default = \"exiftool\" command)" << endl
         << "   -r" << endl
         << "       Specifies if the folder must be explored recursively" << endl;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        display_help();
        return 0;
    }

    omp_set_num_threads(omp_get_num_procs());

    int opt;

    string folder = ".";
    string exiftool;
    bool recursive = false;

    while ((opt = getopt(argc, argv, ":e:f:rh")) != -1) {
        switch (opt) {
            case 'h': // Aide
                display_help();
                return 0;
            case 'e': // Chemin vers exiftool
                exiftool = optarg;
            case 'f': // Dossier à explorer
                folder = optarg;
                break;
            case 'r': // Explorer récursivement
                recursive = true;
                break;
            default:
                break;
        }
    }

    FileManager fm_dir(recursive ? 10000 : 0);

    cout << (recursive ? "recursive " : "") << "directory scan of " << folder << endl;

    fm_dir.scandir_dir(folder);

    map<string, map<string, int>> histogram;

    vector<string> exifs;
    exifs.emplace_back("ApertureValue");
    exifs.emplace_back("ShutterSpeedValue");
    exifs.emplace_back("ISO");
    exifs.emplace_back("FocalLength");
    exifs.emplace_back("LensModel");
    exifs.emplace_back("Model");

    string exifs_string;

    for (const string &exif: exifs) {
        exifs_string.append("-").append(exif).append("\n");
        histogram.insert(pair<string, map<string, int >>(exif, map<string, int>()));
    }

#pragma omp parallel
#pragma omp single
    {
        for (const string &dir : fm_dir.get_dirs()) {
#pragma omp task
            {
                FileManager fm_files(0);
                fm_files.add_allowed_extension("CR2");
                fm_files.add_allowed_extension("ARW");
                fm_files.add_allowed_extension("NEF");
                fm_files.add_allowed_extension("JPG");
                fm_files.add_allowed_extension("JPEG");
                fm_files.scandir_files(dir);
                for (const string &file : fm_files.get_files()) {
#pragma omp task
                    {
                        auto *et = new ExifTool(exiftool.empty() ? nullptr : exiftool.c_str());
                        map<string, bool> seen;
                        seen.clear();
                        for (const string &exif: exifs)
                            seen.insert(pair<string, bool>(exif, false));

                        // read metadata from the image
                        TagInfo *info = et->ImageInfo((dir + "/" + file).c_str(), exifs_string.c_str());
                        if (info) {
                            for (TagInfo *i = info; i; i = i->next) {
                                auto seen_it = seen.find(i->name);
                                if (seen_it != seen.end()) {
                                    // We found a property we wanted
                                    if (!seen.at(i->name)) {
                                        // This property hasn't been processed yet
#pragma omp critical
                                        {
                                            auto ex = histogram.find(i->name);
                                            if (ex != histogram.end()) ex->second[i->value]++;
                                        }
                                    }
                                    seen_it->second = true;
                                }
                            }
                            delete info;
                        } else if (et->LastComplete() <= 0) {
                            cerr << "Error executing exiftool!" << endl;
                        }

                        delete et;
                    }
                }
            }
        }
    }

    for (auto &ex : histogram) {
        cout << ex.first << " : " << endl;

        float sum = 0.0f;

        for (auto &it : ex.second) {
            sum += it.second;
        }

        for (auto &it : ex.second) {
            int percentage = round(100.0f * it.second / sum);
            cout << "  " << it.first << " → " << percentage << "% (" << it.second << ')' << endl
                 << "\t\t\t[";
            int i;
            for (i = 0; i < percentage / 2; i++)cout << '*';
            for (; i < 50; i++)cout << ' ';
            cout << ']' << endl;
        }

    }

    return 0;
}
