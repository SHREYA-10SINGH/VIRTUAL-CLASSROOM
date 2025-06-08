// VisualVirtualClassroom.cpp
// Standalone C++ console virtual classroom implementing MVC pattern
//
// Design uses ASCII UI with clear spacing, "cards" via unicode box drawing,
// clean typography via spacing and text alignment, and interaction via console prompts.
//
// Data persistence simulated via text files in project directory.
//
// Author: BLACKBOXAI

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <cstdio> // for remove in clear screen
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// ANSI console color codes for gray text and emphasis (some terminals support)
#define COLOR_RESET   "\033[0m"
#define COLOR_GRAY    "\033[38;2;107;114;128m"  // #6b7280 neutral gray
#define COLOR_BOLD    "\033[1m"

// Helper to clear screen cross platform
void ClearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Model: Manages data storage for classes and students
class Model {
public:
    Model() {
        LoadData();
    }

    // Add a class if name is unique
    bool AddClass(const std::string& className) {
        if (!Exists(classes, className)) {
            classes.push_back(className);
            SaveData();
            return true;
        }
        return false;
    }

    // Add a student if name is unique
    bool AddStudent(const std::string& studentName) {
        if (!Exists(students, studentName)) {
            students.push_back(studentName);
            SaveData();
            return true;
        }
        return false;
    }

    const std::vector<std::string>& GetClasses() const { return classes; }
    const std::vector<std::string>& GetStudents() const { return students; }

private:
    std::vector<std::string> classes;
    std::vector<std::string> students;

    bool Exists(const std::vector<std::string>& vec, const std::string& val) {
        return std::find(vec.begin(), vec.end(), val) != vec.end();
    }

    void LoadData() {
        // Load classes from "classes.txt"
        std::ifstream finClasses("classes.txt");
        if (finClasses.is_open()) {
            std::string line;
            while (std::getline(finClasses, line)) {
                Trim(line);
                if (!line.empty()) classes.push_back(line);
            }
            finClasses.close();
        }
        // Load students from "students.txt"
        std::ifstream finStudents("students.txt");
        if (finStudents.is_open()) {
            std::string line;
            while (std::getline(finStudents, line)) {
                Trim(line);
                if (!line.empty()) students.push_back(line);
            }
            finStudents.close();
        }
    }

    void SaveData() {
        // Save classes
        std::ofstream foutClasses("classes.txt", std::ios::trunc);
        for (const auto& c : classes) foutClasses << c << '\n';
        foutClasses.close();

        // Save students
        std::ofstream foutStudents("students.txt", std::ios::trunc);
        for (const auto& s : students) foutStudents << s << '\n';
        foutStudents.close();
    }

    // Trim helper
    static void Trim(std::string& s) {
        const char* whitespace = " \t\n\r\f\v";
        s.erase(s.find_last_not_of(whitespace) + 1);
        s.erase(0, s.find_first_not_of(whitespace));
    }
};

// View: Manages all console output and input UI
class View {
public:
    View() = default;

    // Display header with app name and top nav
    void DisplayHeader() {
        ClearScreen();
        using namespace std;
        cout << COLOR_BOLD;
        cout << "=============================================\n";
        cout << "                VCLASS 1.0                   \n";
        cout << "=============================================\n" << COLOR_RESET;
        cout << "\n";
        cout << COLOR_GRAY;
        cout << "1. Add Class     2. Add Student     3. View Classes\n";
        cout << "4. View Students 5. Quit\n";
        cout << COLOR_RESET << "\n";
    }

    // Prompt user for menu choice
    int PromptMainMenuChoice() {
        std::cout << "Choose an option (1-5): ";
        int choice = 0;
        while (!(std::cin >> choice) || choice < 1 || choice > 5) {
            std::cout << "Invalid input. Enter 1-5: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // flush newline
        return choice;
    }

    // Prompt for a non-empty string with a label, return result trimmed
    std::string PromptNonEmptyString(const std::string& prompt) {
        std::string input;
        do {
            std::cout << prompt;
            std::getline(std::cin, input);
            Trim(input);
            if (input.empty()) std::cout << "Input cannot be empty. Try again.\n";
        } while (input.empty());
        return input;
    }

    // Display a "card"-style box with title and content lines
    void DisplayCard(const std::string& title, const std::vector<std::string>& lines, int cardWidth = 50) {
        using namespace std;
        const std::string topBot = "╭" + std::string(cardWidth - 2, '_') + "╮\n";
        const std::string bottom = "╰" + std::string(cardWidth - 2, '_') + "╯\n";

        std::cout << COLOR_GRAY << topBot << COLOR_RESET;

        // Title line centered
        std::cout << "│" << std::string((cardWidth - 2 - title.size()) / 2, ' ')
                  << COLOR_BOLD << title << COLOR_RESET
                  << std::string((cardWidth - 2 - title.size() + 1) / 2, ' ') << "│\n";

        std::cout << "│" << std::string(cardWidth - 2, ' ') << "│\n";

        // Content lines wrapped and padded
        for (const auto& line : lines) {
            std::cout << "│ " << line;
            int padding = cardWidth - 3 - line.size();
            std::cout << std::string(padding > 0 ? padding : 0, ' ') << "│\n";
        }

        std::cout << COLOR_GRAY << bottom << COLOR_RESET;
    }

    // Display list of cards in a grid style (2 per row)
    void DisplayCardsGrid(const std::vector<std::pair<std::string, std::vector<std::string>>>& cards) {
        int cardsPerRow = 2;
        int cardWidth = 50;
        size_t count = cards.size();
        for (size_t i = 0; i < count; i += cardsPerRow) {
            // collect line-by-line output for all cards in row to print side by side
            // cards have 7 lines each (top+title+blank+3 content+bot)

            std::vector<std::string> lines[7];

            for (int c = 0; c < cardsPerRow; ++c) {
                size_t idx = i + c;
                if (idx >= count) break;
                const auto& card = cards[idx];

                // Compose card lines
                lines[0].push_back("╭" + std::string(cardWidth - 2, '_') + "╮");
                int titlePadLeft = (cardWidth - 2 - card.first.size()) / 2;
                int titlePadRight = cardWidth - 2 - card.first.size() - titlePadLeft;
                std::string titleLine = "│" + std::string(titlePadLeft, ' ') + COLOR_BOLD + card.first + COLOR_RESET + std::string(titlePadRight, ' ') + "│";
                lines[1].push_back(titleLine);
                lines[2].push_back("│" + std::string(cardWidth - 2, ' ') + "│");

                // Up to 3 content lines, pad shorter lines with spaces
                int contentLines = 3;
                for (int ln = 0; ln < contentLines; ++ln) {
                    if (ln < (int)card.second.size()) {
                        std::string content = card.second[ln];
                        if ((int)content.size() > cardWidth - 3) // truncate content if too long
                            content = content.substr(0, cardWidth - 6) + "...";
                        int padRight = cardWidth - 3 - content.size();
                        lines[3 + ln].push_back("│ " + content + std::string(padRight, ' ') + "│");
                    } else {
                        lines[3 + ln].push_back("│" + std::string(cardWidth - 2, ' ') + "│");
                    }
                }

                lines[6].push_back("╰" + std::string(cardWidth - 2, '_') + "╯");
            }

            // Print lines side by side separated by 4 spaces
            for (int lineNum = 0; lineNum < 7; ++lineNum) {
                for (size_t cardIdx = 0; cardIdx < lines[lineNum].size(); ++cardIdx) {
                    std::cout << COLOR_GRAY << lines[lineNum][cardIdx] << COLOR_RESET;
                    if (cardIdx != lines[lineNum].size() - 1)
                        std::cout << "    "; // spacing between cards
                }
                std::cout << "\n";
            }
            std::cout << "\n";
        }
    }

    // Display hero section with big headline, subtext and prompt
    void DisplayHero() {
        std::cout << COLOR_BOLD;
        std::cout << "\n======================== WELCOME TO VCLASS ========================\n\n";
        std::cout << COLOR_RESET;
        std::cout << COLOR_GRAY;
        std::cout << "Create and manage your virtual classes and students with ease.\n\n";
        std::cout << COLOR_RESET;
    }

    // Display footer
    void DisplayFooter() {
        std::cout << COLOR_GRAY << "====================================================================\n";
        std::cout << "                   © 2024 VClass Virtual Classroom                  \n";
        std::cout << "====================================================================" << COLOR_RESET << "\n\n";
    }

    // Pause prompt
    void Pause() {
        std::cout << "Press Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

private:
    static void Trim(std::string& s) {
        const char* ws = " \t\n\r\f\v";
        s.erase(s.find_last_not_of(ws) + 1);
        s.erase(0, s.find_first_not_of(ws));
    }
};

// Controller: orchestrates program flow
class Controller {
public:
    Controller(): model(), view() {}

    void Run() {
        view.DisplayHero();
        bool running = true;
        while (running) {
            view.DisplayHeader();
            int choice = view.PromptMainMenuChoice();
            switch(choice) {
                case 1: AddClassFlow(); break;
                case 2: AddStudentFlow(); break;
                case 3: ViewClassesFlow(); break;
                case 4: ViewStudentsFlow(); break;
                case 5: running = false; break;
            }
        }
        view.DisplayFooter();
    }

private:
    Model model;
    View view;

    void AddClassFlow() {
        std::string name = view.PromptNonEmptyString("Enter new class name: ");
        if (model.AddClass(name)) {
            std::cout << "\nClass \"" << name << "\" added successfully.\n\n";
        } else {
            std::cout << "\nClass \"" << name << "\" already exists.\n\n";
        }
        view.Pause();
    }

    void AddStudentFlow() {
        std::string name = view.PromptNonEmptyString("Enter new student name: ");
        if (model.AddStudent(name)) {
            std::cout << "\nStudent \"" << name << "\" added successfully.\n\n";
        } else {
            std::cout << "\nStudent \"" << name << "\" already exists.\n\n";
        }
        view.Pause();
    }

    void ViewClassesFlow() {
        auto classes = model.GetClasses();
        if (classes.empty()) {
            std::cout << "\nNo classes available.\n\n";
        } else {
            std::vector<std::pair<std::string, std::vector<std::string>>> cards;
            for (const auto& c : classes) {
                cards.emplace_back(c, std::vector<std::string>{"Manage and track your class activities."});
            }
            std::cout << "\n--- Classes ---\n";
            view.DisplayCardsGrid(cards);
        }
        view.Pause();
    }

    void ViewStudentsFlow() {
        auto students = model.GetStudents();
        if (students.empty()) {
            std::cout << "\nNo students enrolled.\n\n";
        } else {
            std::vector<std::pair<std::string, std::vector<std::string>>> cards;
            for (const auto& s : students) {
                cards.emplace_back(s, std::vector<std::string>{"Active participant in your classes."});
            }
            std::cout << "\n--- Students ---\n";
            view.DisplayCardsGrid(cards);
        }
        view.Pause();
    }
};

int main() {
    Controller app;
    app.Run();
    return 0;
}
