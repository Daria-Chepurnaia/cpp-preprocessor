#pragma once 

#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using filesystem::path;

// Custom literal for path
path operator""_p(const char* data, std::size_t sz) ;

// Function to handle includes and preprocess the file
bool ProcessInclude(const string& line, const path& in_file, ostream& ostr, const vector<path>& include_directories, int current_line);

// Function to handle local includes
bool HandleLocalInclude(const smatch& file_match, const path& in_file, ostream& ostr, const vector<path>& include_directories, int current_line);

// Function to handle library includes
bool HandleLibraryInclude(const smatch& lib_match, ostream& ostr, const vector<path>& include_directories, int current_line);

// Recursively preprocesses the input file
bool RecPreprocess(const path& in_file, ostream& ostr, const vector<path>& include_directories);

// Function to process includes in a line
bool ProcessInclude(const string& line, const path& in_file, ostream& ostr, const vector<path>& include_directories, int current_line);

// Function to handle local include files
bool HandleLocalInclude(const smatch& file_match, const path& in_file, ostream& ostr, const vector<path>& include_directories, int current_line);

// Function to handle library include files
bool HandleLibraryInclude(const smatch& lib_match, ostream& ostr, const vector<path>& include_directories, int current_line);

// Preprocess function to initialize file reading and writing
bool Preprocess(const path& in_file, const path& out_file, const vector<path>& include_directories);

// Function to read file contents into a string
string GetFileContents(const string& file);
