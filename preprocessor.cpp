#include "preprocessor.h"

// Custom literal for path
path operator""_p(const char* data, std::size_t sz) {
    return path(data, data + sz);
}

// Recursively preprocesses the input file
bool RecPreprocess(const path& in_file, ostream& ostr, const vector<path>& include_directories) {
    ifstream input(in_file);
    if (!input.is_open()) {
        cerr << "Failed to open input file: " << in_file.string() << endl;
        return false;
    }

    string line;
    int current_line = 1;

    while (getline(input, line)) {
        // Process the line to handle includes
        if (!ProcessInclude(line, in_file, ostr, include_directories, current_line)) {
            return false;
        }
        current_line++;
    }

    input.close();
    return true;
}

// Function to process includes in a line
bool ProcessInclude(const string& line, const path& in_file, ostream& ostr, const vector<path>& include_directories, int current_line) {
    static regex find_file(R"/(\s*#\s*include\s*"([^"]*)"\s*)/");
    static regex find_lib(R"/(\s*#\s*include\s*<([^>]*)>\s*)/");

    smatch file_match, lib_match;

    if (regex_match(line, file_match, find_file)) {
        return HandleLocalInclude(file_match, in_file, ostr, include_directories, current_line);
    } else if (regex_match(line, lib_match, find_lib)) {
        return HandleLibraryInclude(lib_match, ostr, include_directories, current_line);
    } else {
        ostr << line << endl; // Not an include, just output the line
        return true;
    }
}

// Function to handle local include files
bool HandleLocalInclude(const smatch& file_match, const path& in_file, ostream& ostr, const vector<path>& include_directories, int current_line) {
    path included_file = string(file_match[1]);
    path file_path = in_file.parent_path() / included_file;

    if (filesystem::exists(file_path)) {
        // Recursively process the included file
        return RecPreprocess(file_path, ostr, include_directories);
    }

    // Check in include directories
    for (const auto& dir : include_directories) {
        path dir_path = dir / included_file;
        if (filesystem::exists(dir_path)) {
            // Recursively process the included file found in directory
            return RecPreprocess(dir_path, ostr, include_directories);
        }
    }

    cerr << "Unknown include file " << included_file.filename().string() << " at " 
         << in_file.string() << " at line " << current_line << endl;
    return false;
}

// Function to handle library include files
bool HandleLibraryInclude(const smatch& lib_match, ostream& ostr, const vector<path>& include_directories, int current_line) {
    path included_file = string(lib_match[1]);

    // Check in include directories for library file
    for (const auto& dir : include_directories) {
        path dir_path = dir / included_file;
        if (filesystem::exists(dir_path)) {
            // Recursively process the included library file found in directory
            return RecPreprocess(dir_path, ostr, include_directories);
        }
    }

    cerr << "Unknown library include file " << included_file.filename().string() << " at line " 
         << current_line << endl;
    return false;
}

// Preprocess function to initialize file reading and writing
bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories) {
    ifstream input(in_file);
    if (!input.is_open()) {
        cerr << "Failed to open input file: " << in_file.string() << endl;
        return false;
    }

    ofstream output(out_file);
    if (!output.is_open()) {
        cerr << "Failed to open output file: " << out_file.string() << endl;
        return false;
    }

    return RecPreprocess(in_file, output, include_directories);
}

// Function to read file contents into a string
string GetFileContents(const string& file) {
    ifstream stream(file);
    return { istreambuf_iterator<char>(stream), istreambuf_iterator<char>() };
}

