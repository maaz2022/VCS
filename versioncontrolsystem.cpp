#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <chrono>
#include <ctime>
#include <sstream>
#include <functional>

namespace fs = std::filesystem;
// this function for calculating hash
std::size_t hashCal(const std::string& str) {
    return std::hash<std::string>{}(str);
}
// this function is for simple creating file
void fileCreate(const std::string& fileName) {
    std::ofstream file(fileName);
    if (file.is_open()) {
        std::cout << "File has been created: " << fileName << std::endl;
    } else {
        std::cerr << "File is not created: " << fileName << std::endl;
    }
}
// this function is for simple creating repo
void createRepo(const std::string& repoName) {
    if (fs::create_directory(repoName)) {
        std::cout << "Repository created: " << repoName << std::endl;
    } else {
        std::cerr << "Repository is not created: " << repoName << std::endl;
    }
}
// this function is for tracking files within the repository

void trackFilesInRepo(const std::string& repoName) {
    for (const auto& entry : fs::directory_iterator(repoName)) {
        std::cout << entry.path() << std::endl;
    }
}
// this function is for commit changes
void commitChanges(const std::string& repoName, std::vector<std::string>& changedFiles, std::vector<std::size_t>& fileHashes, int version) {
    if (changedFiles.empty()) {
        std::cout << "No changes to commit." << std::endl;
        return;
    }
    std::string commitFileName = repoName + "/commit.txt";
    std::ofstream commitFile(commitFileName, std::ios::app);
    if (commitFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

        //  coversion ------current time to string
        std::string timeStr = std::asctime(std::localtime(&currentTime));

        commitFile << "Version " << version << " - Commit at " << timeStr; 
        for (const auto& file : changedFiles) {
            commitFile << " - " << file << std::endl;
            std::ifstream inputFile(repoName + "/" + file);
            if (inputFile.is_open()) {
                std::string line;
                std::stringstream content;
                while (std::getline(inputFile, line)) {
                    content << line << std::endl;
                }
                inputFile.close();

            //  #here for each content hash is calculated
                std::size_t hash = hashCal(content.str());
                if (std::find(fileHashes.begin(), fileHashes.end(), hash) == fileHashes.end()) {
                    fileHashes.push_back(hash);
                    commitFile << content.str() << std::endl;
                    commitFile << "Hash: " << hash << std::endl;
                }
            } else {
                std::cerr << "Unable to open file: " << repoName + "/" + file << std::endl;
            }
        }
        std::cout << "Changes committed successfully." << std::endl;
    } else {
        std::cerr << "Unable to open commit file: " << commitFileName << std::endl;
    }
}

// this function is for showing the commit log
void viewCommitLog(const std::string& repoName) {
    std::string commitFileName = repoName + "/commit.txt";
    std::ifstream commitFile(commitFileName);
    if (commitFile.is_open()) {
        std::string line;
        while (std::getline(commitFile, line)) {
            std::cout << line << std::endl;
        }
    } else {
        std::cerr << "Unable to open commit file: " << commitFileName << std::endl;
    }
}
// this function is for revert to previos version of the files
void revertToFileVersion(const std::string& repoName, const std::string& fileName, int version, const std::vector<std::size_t>& fileHashes) {
    std::string commitFileName = repoName + "/commit.txt";
    std::ifstream commitFile(commitFileName);
    if (!commitFile.is_open()) {
        std::cerr << "Unable to open commit file: " << commitFileName << std::endl;
        return;
    }

    std::vector<std::string> fileContent; // 

    std::string line;
    int currentVersion = 0;
    bool found = false;
    while (std::getline(commitFile, line)) {
        if (line.find("Version") != std::string::npos) {
            currentVersion = std::stoi(line.substr(8)); 
        } else if (currentVersion == version) {
            if (line.find(fileName) != std::string::npos) {
                found = true;
            } else if (found) {
                fileContent.push_back(line); 
            }
        }
    }

    commitFile.close();

    if (!found) {
        std::cout << "Version " << version << " of file '" << fileName << "' not found in commit log." << std::endl;
        return;
    }

    std::ofstream outputFile(repoName + "/" + fileName);
    if (!outputFile.is_open()) {
        std::cerr << "Unable to open file for writing: " << repoName + "/" + fileName << std::endl;
        return;
    }


    for (const auto& content : fileContent) {
        outputFile << content << std::endl;
    }

    outputFile.close();

    std::cout << "File reverted to version " << version << "." << std::endl;
}

void addContentToFile(const std::string& fileName) {
    std::ofstream file(fileName, std::ios::app);
    if (file.is_open()) {
        std::string content;
        std::cout << "Enter content to add to the file (enter '.' on a new line to finish):" << std::endl;
        std::cin.ignore(); 
        while (true) {
            std::getline(std::cin, content);
            if (content == ".")
                break;
            file << content << std::endl;
        }
        std::cout << "Content added to the file." << std::endl;
    } else {
        std::cerr << "Unable to open file: " << fileName << std::endl;
    }
}

int main() {
    std::string repoName;
    std::string command;
    std::vector<std::string> changedFiles;
    std::vector<std::size_t> fileHashes;
    int version = 1; 

    while (true) {
        std::cout << "Enter folder name or enter 'exit' to quit: ";
        std::cin >> repoName;
        
        if (repoName == "exit")
            break;
        
        if (!fs::exists(repoName)) {
            std::cout << "Folder does not exist. Do you want to create it? (yes/no): ";
            std::string response;
            std::cin >> response;
            if (response == "yes") {
                createRepo(repoName);
            } else {
                std::cerr << "Folder does not exist!" << std::endl;
                continue;
            }
        }

        while (true) {
            std::cout << "input what Operation you want to Perform (1: create file, 2: track files, 3: commit changes, 4: add content to file, 5: view commit log, 6: revert to file version, 7: change folder, 8: exit): ";
            std::cin >> command;

            switch (std::stoi(command)) {
                case 1: {
                    std::string fileName;
                    std::cout << "Enter file name: ";
                    std::cin >> fileName;
                    fileCreate(repoName + "/" + fileName);
                    changedFiles.push_back(fileName);
                    break;
                }
                case 2: {
                    trackFilesInRepo(repoName);
                    break;
                }
                case 3: {
                    commitChanges(repoName, changedFiles, fileHashes, version++);
                 
                    changedFiles.clear();
                    break;
                }
                case 4: {
                    std::string fileName;
                    std::cout << "Enter file name to add content: ";
                    std::cin >> fileName;
                    addContentToFile(repoName + "/" + fileName);
                    changedFiles.push_back(fileName); 
                    break;
                }
                case 5: {
                    viewCommitLog(repoName);
                    break;
                }
                case 6: {
                    std::string fileName;
                    std::cout << "Enter file name to revert: ";
                    std::cin >> fileName;
                    int ver;
                    std::cout << "Enter version number to revert to: ";
                    std::cin >> ver;
                    revertToFileVersion(repoName, fileName, ver, fileHashes);
                    break;
                }
                case 7: {
                    std::string newrepoName;
                    std::cout << "Enter new Repo name: ";
                    std::cin >> newrepoName;
                    createRepo(newrepoName);
                    repoName = newrepoName;
                    break;
                }
                case 8: {
                    return 0;
                }
                default:
                    std::cerr << "Invalid command!" << std::endl;
                    break;
            }
        }
    }

    return 0;
}
