/*
 * MINI QUIZ GAME - SAKURA EDITION (Multiplayer Classroom Platform Core)
 * 
 * Preserves original C++ project concepts:
 * - Player, Quiz Category, Question, Quiz Session
 * - Answer Record, Score Record, Leaderboard, Question Bank, Score File
 * - 9 Categories, 10 Questions, 180s Timer, 20 Marks Scoring System
 * - High-precision tie-breaking ranking (Marks first, completion time tie-breaker)
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <iomanip>
#include <random>
#include <limits>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#define CLEAR "cls"
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#else
#define CLEAR "clear"
#endif

// ===== ANSI Color Codes & Sakura Theme Palettes =====
#define RST   "\033[0m"
#define BOLD  "\033[1m"
#define DIM   "\033[2m"
#define ITAL  "\033[3m"

#define SAKURA_PINK   "\033[38;2;255;182;193m"
#define SAKURA_ROSE   "\033[38;2;255;105;180m"
#define SAKURA_DARK   "\033[38;2;219;112;147m"
#define SAKURA_PALE   "\033[38;2;255;228;225m"
#define LANTERN_GOLD  "\033[38;2;255;215;0m"
#define SHRINE_RED    "\033[38;2;220;20;60m"
#define MYSTIC_PURPLE "\033[38;2;147;112;219m"
#define NIGHT_INDIGO  "\033[38;2;106;90;205m"

#define RED   "\033[31m"
#define GRN   "\033[32m"
#define YEL   "\033[33m"
#define BLU   "\033[34m"
#define MAG   "\033[35m"
#define CYN   "\033[36m"
#define WHT   "\033[37m"
#define BWHT  "\033[97m"

#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"

// ===== Core Data Structures =====

struct Question {
    int id;
    std::string difficulty; // "Easy", "Medium", "Hard", "Expert"
    int marks;              // 1, 1, 1, 1, 1, 2, 2, 3, 4, 5 (Total = 20)
    std::string text;
    std::vector<std::string> options;
    int correctIndex;
    std::string explanation;
};

struct AnswerRecord {
    int questionIndex;
    int chosenIndex;
    int correctIndex;
    int marksAwarded;
    bool isCorrect;
};

struct ScoreRecord {
    std::string playerName;
    std::string category;
    int score;             // marks obtained (out of 20)
    int maxScore;          // 20
    double completionTime; // seconds
    int correctCount;
    int incorrectCount;
    int unansweredCount;
    std::string date;
    int rank;
};

struct QuizCategory {
    std::string name;
    std::string japaneseName;
    std::string icon;
    std::string color;
    std::vector<Question> (*loader)();
};

// ===== Utility Functions =====

void sleepMs(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void enableVT() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

void clearScreen() { 
    system(CLEAR); 
}

std::string currentDate() {
    time_t t = time(nullptr);
    char buf[30];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", localtime(&t));
    return std::string(buf);
}

void printCentered(const std::string& s, int width = 74, const std::string& color = BWHT) {
    int pad = (width - (int)s.size()) / 2;
    if (pad < 0) pad = 0;
    std::cout << color << std::string(pad, ' ') << s << RST << "\n";
}

void printSakuraBar(int width = 74, const std::string& pattern = "~*~", const std::string& color = SAKURA_ROSE) {
    std::cout << color;
    int reps = width / 3;
    for (int i = 0; i < reps; ++i) std::cout << pattern;
    std::cout << RST << "\n";
}

void typeWrite(const std::string& msg, int delayMs = 15, const std::string& color = SAKURA_PALE) {
    std::cout << color;
    for (char c : msg) {
        std::cout << c << std::flush;
        sleepMs(delayMs);
    }
    std::cout << RST;
}

// ===== Animated Falling Sakura Petal Effect =====

void sakuraFallingEffect(int frames = 6) {
    const char* layers[] = {
        "   .   *   .  *  sakura petals drifting in the breeze  *   .   *   .  ",
        " *   .   *   .   ( 桜の花びらが舞い散る )   .   *   .   *   .   * ",
        "   *   .   *   .   *   .   *   .   *   .   *   .   *   .   *   .   "
    };
    for (int f = 0; f < frames; ++f) {
        std::cout << "\r" << SAKURA_PINK << layers[f % 3] << RST << std::flush;
        sleepMs(120);
    }
    std::cout << "\r" << std::string(75, ' ') << "\r";
}

// ===== Splash Screen (Sakura Aesthetic) =====

void splashScreen() {
    clearScreen();
    std::cout << HIDE_CURSOR << "\n";

    std::vector<std::string> torii = {
        "        |===================================================|        ",
        "        |   *  *   MINI QUIZ GAME - SAKURA EDITION   *  *   |        ",
        "        |===================================================|        ",
        "           | |   [ 桜 ]   CLASSROOM MULTIPLAYER ARENA   | |          ",
        "          /   \\                                        /   \\         ",
        "         |     |      Real-Time 180s - 20 Marks       |     |        ",
        "         |     |      Live Leaderboard - 9 Arenas     |     |        "
    };

    for (const auto& line : torii) {
        printCentered(line, 74, SAKURA_PINK);
        sleepMs(40);
    }

    std::cout << "\n";
    printSakuraBar(74, "*~*", SAKURA_ROSE);
    printCentered("J A P A N E S E   S A K U R A   K N O W L E D G E   T R I A L", 74, BWHT);
    printSakuraBar(74, "*~*", SAKURA_ROSE);
    std::cout << "\n";

    typeWrite("  Initializing Sakura Quiz Engine & Question Bank...", 20, SAKURA_PALE);
    std::cout << "\n  [";
    for (int i = 0; i < 30; ++i) {
        sleepMs(25);
        if (i < 10) std::cout << SAKURA_PINK << "#" << RST;
        else if (i < 20) std::cout << SAKURA_ROSE << "#" << RST;
        else std::cout << LANTERN_GOLD << "#" << RST;
        std::cout << std::flush;
    }
    std::cout << "] " << SAKURA_PALE << BOLD << "READY" << RST << "\n\n";

    sleepMs(300);
    std::cout << SHOW_CURSOR;
}

// ===== Question Banks for All 9 Categories =====
// 10 Questions each: Q1-Q5 (Easy, 1m), Q6-Q7 (Med, 2m), Q8 (Hard, 3m), Q9 (Hard, 4m), Q10 (Expert, 5m)

std::vector<Question> scienceQuestions() {
    return {
        {1, "Easy", 1, "What is the chemical symbol for Gold in the periodic table?", {"Ag", "Au", "Fe", "Cu"}, 1, "Au comes from the Latin 'aurum', meaning shining dawn."},
        {2, "Easy", 1, "Which planet in our solar system is known as the Red Planet?", {"Venus", "Jupiter", "Mars", "Saturn"}, 2, "Mars appears red due to iron oxide rust on its surface."},
        {3, "Easy", 1, "What organelle is called the 'powerhouse of the cell'?", {"Nucleus", "Ribosome", "Mitochondria", "Golgi apparatus"}, 2, "Mitochondria generate ATP energy via cellular respiration."},
        {4, "Easy", 1, "Which subatomic particle has a negative electric charge?", {"Proton", "Neutron", "Positron", "Electron"}, 3, "Electrons carry a negative elementary charge of -1.602 x 10^-19 C."},
        {5, "Easy", 1, "What gas do plants primarily absorb during photosynthesis?", {"Oxygen", "Nitrogen", "Carbon Dioxide", "Hydrogen"}, 2, "Plants absorb CO2 and water to produce glucose and oxygen."},
        {6, "Medium", 2, "What phenomenon causes daylight sky to appear blue?", {"Mie scattering", "Rayleigh scattering", "Refraction", "Ozone absorption"}, 1, "Rayleigh scattering scatters shorter blue wavelengths much more than red."},
        {7, "Medium", 2, "Which metal has the highest melting point of all elements (3,422 deg C)?", {"Titanium", "Platinum", "Tungsten", "Osmium"}, 2, "Tungsten has an exceptional melting point of 3,422 deg C."},
        {8, "Hard", 3, "What is the half-life of Carbon-14 used in radiocarbon dating?", {"~570 years", "~5,730 years", "~57,300 years", "~573,000 years"}, 1, "Carbon-14 decays with a half-life of ~5,730 years."},
        {9, "Hard", 4, "Which quantum effect allows electrons to pass through energy barriers without sufficient classical energy?", {"Wave interference", "Superposition", "Quantum tunneling", "Photoelectric effect"}, 2, "Quantum tunneling permits electrons to cross finite barriers in flash memory and stars."},
        {10, "Expert", 5, "What fraction of the universe's total energy density is theorized to be Dark Energy?", {"~5%", "~27%", "~68%", "~95%"}, 2, "Dark Energy accounts for ~68% of the universe, causing accelerated expansion."}
    };
}

std::vector<Question> techQuestions() {
    return {
        {1, "Easy", 1, "What does 'CPU' stand for in computer hardware architecture?", {"Central Processing Unit", "Core Power Utility", "Computer Program Unit", "Central Peripheral Union"}, 0, "The CPU executes program instructions and directs computations."},
        {2, "Easy", 1, "Which language was created in 1995 for interactive web browsers?", {"Python", "Java", "JavaScript", "C++"}, 2, "Brendan Eich created JavaScript in 10 days at Netscape in 1995."},
        {3, "Easy", 1, "What protocol underlies client-server communication on the web?", {"FTP", "HTTP", "SMTP", "SSH"}, 1, "HyperText Transfer Protocol is the foundation of data communication on the Web."},
        {4, "Easy", 1, "What decimal number is represented by 4-bit binary 1010?", {"8", "10", "12", "14"}, 1, "1x8 + 0x4 + 1x2 + 0x1 = 10."},
        {5, "Easy", 1, "In Git, which command creates and switches to a new branch?", {"git push -b", "git branch -n", "git checkout -b", "git merge --new"}, 2, "git checkout -b (or git switch -c) creates and activates the new branch."},
        {6, "Medium", 2, "What is the guaranteed worst-case time complexity of Merge Sort?", {"O(n)", "O(n log n)", "O(n^2)", "O(log n)"}, 1, "Merge sort guarantees O(n log n) by consistently dividing and merging."},
        {7, "Medium", 2, "Which cryptographic principle guarantees blockchain immutability?", {"Zero-knowledge proofs", "Cryptographic Hash Chaining", "Symmetric encryption", "Diffie-Hellman"}, 1, "Each block stores the previous block's SHA-256 hash in a tamper-evident chain."},
        {8, "Hard", 3, "According to the CAP Theorem, which 3 properties cannot be simultaneously guaranteed?", {"Concurrency, Atomicity, Performance", "Consistency, Availability, Partition Tolerance", "Correctness, Authentication, Privacy", "Caching, Availability, Persistence"}, 1, "Under partitions, a distributed system must sacrifice either immediate consistency or continuous availability."},
        {9, "Hard", 4, "What core mechanism in Transformer neural networks enables parallel token processing?", {"Recurrent loops", "Multi-Head Self-Attention", "Convolutional pooling", "Stochastic dropout"}, 1, "Self-attention computes pairwise relevance weights across all tokens concurrently."},
        {10, "Expert", 5, "What compiler optimization reuses stack frames when the final action is recursion?", {"Loop unrolling", "Tail Call Optimization (TCO)", "Dead code elimination", "Constant folding"}, 1, "TCO converts tail recursion into an O(1) jump, preventing stack overflow."}
    };
}

std::vector<Question> mathQuestions() {
    return {
        {1, "Easy", 1, "What is the mathematical constant Pi (pi) to two decimal places?", {"3.12", "3.14", "3.16", "3.18"}, 1, "Pi is the ratio of circumference to diameter (~3.14159...)."},
        {2, "Easy", 1, "What is the square root of 144?", {"10", "11", "12", "14"}, 2, "12 x 12 = 144."},
        {3, "Easy", 1, "In Euclidean geometry, what is the sum of interior angles in any triangle?", {"90 deg", "180 deg", "270 deg", "360 deg"}, 1, "The three interior angles of any triangle on a Euclidean plane sum to 180 deg."},
        {4, "Easy", 1, "What is the only even prime number in mathematics?", {"0", "2", "4", "6"}, 1, "2 is the only even prime because all larger even numbers are divisible by 2."},
        {5, "Easy", 1, "Solve for x in: 3x + 9 = 24", {"3", "4", "5", "6"}, 2, "3x = 15 => x = 5."},
        {6, "Medium", 2, "What is the sum of interior angles of a regular 6-sided hexagon?", {"540 deg", "720 deg", "900 deg", "1080 deg"}, 1, "(6 - 2) x 180 = 4 x 180 = 720 degrees."},
        {7, "Medium", 2, "If two fair dice are rolled, what is the probability of summing to 7?", {"1/12", "1/6", "5/36", "7/36"}, 1, "6 winning combinations out of 36 outcomes = 6/36 = 1/6."},
        {8, "Hard", 3, "In the Fibonacci sequence (1,1,2,3,5,8,13,21,34,...), what is the 10th term?", {"45", "55", "89", "144"}, 1, "The 10th Fibonacci number is 55."},
        {9, "Hard", 4, "What is the Hardy-Ramanujan number, smallest sum of two cubes in two ways?", {"128", "1024", "1729", "2048"}, 2, "1729 = 1^3 + 12^3 = 9^3 + 10^3, identified by Ramanujan to Hardy."},
        {10, "Expert", 5, "What does the Riemann Hypothesis state about non-trivial zeros of the Zeta function?", {"They are all integers", "Their real part is precisely 1/2", "They diverge to infinity", "They match primes"}, 1, "All non-trivial zeros are conjectured to lie on the line Re(s) = 1/2."}
    };
}

std::vector<Question> historyQuestions() {
    return {
        {1, "Easy", 1, "Who was the first President of the United States?", {"Thomas Jefferson", "John Adams", "George Washington", "Alexander Hamilton"}, 2, "George Washington was elected first President in 1789."},
        {2, "Easy", 1, "In which year did World War II end?", {"1943", "1944", "1945", "1948"}, 2, "World War II concluded in 1945."},
        {3, "Easy", 1, "Which ancient civilization built the Pyramids of Giza?", {"Ancient Greece", "Ancient Egypt", "Babylonia", "Persian Empire"}, 1, "The Egyptian Fourth Dynasty pharaohs built the Giza pyramids ~4,500 years ago."},
        {4, "Easy", 1, "Who painted the famous Renaissance portrait Mona Lisa?", {"Michelangelo", "Raphael", "Leonardo da Vinci", "Donatello"}, 2, "Leonardo da Vinci painted the Mona Lisa in Florence."},
        {5, "Easy", 1, "The Roman Colosseum was primarily used for what public events?", {"Plays", "Gladiatorial games & spectacles", "Elections", "Olympics"}, 1, "The Colosseum hosted gladiatorial contests and public spectacles."},
        {6, "Medium", 2, "The Magna Carta limiting royal power was signed in 1215 by which king?", {"Richard I", "King John", "Henry III", "Edward I"}, 1, "King John of England was forced to grant the Magna Carta at Runnymede."},
        {7, "Medium", 2, "The fall of Constantinople in 1453 marked the end of which empire?", {"Holy Roman", "Byzantine Empire", "Sassanid", "Ottoman"}, 1, "Constantinople fell to Mehmed II, ending the 1,100-year Byzantine Empire."},
        {8, "Hard", 3, "Which self-governing nation first granted women the right to vote in 1893?", {"USA", "UK", "New Zealand", "Finland"}, 2, "New Zealand granted adult women the vote in September 1893."},
        {9, "Hard", 4, "The Rosetta Stone contained decrees carved in how many scripts?", {"Two", "Three", "Four", "Five"}, 1, "Three scripts: Hieroglyphic, Demotic, and Ancient Greek."},
        {10, "Expert", 5, "The Peace of Westphalia (1648) ending state conflicts concluded which war?", {"Hundred Years' War", "Thirty Years' War", "War of Spanish Succession", "Seven Years' War"}, 1, "The 1648 peace treaties ended the Thirty Years' War and established nation-state sovereignty."}
    };
}

std::vector<Question> geographyQuestions() {
    return {
        {1, "Easy", 1, "What is the largest ocean on Earth by surface area?", {"Atlantic", "Indian", "Arctic", "Pacific Ocean"}, 3, "The Pacific Ocean covers over 165 million km2."},
        {2, "Easy", 1, "What is the capital city of Japan?", {"Kyoto", "Osaka", "Tokyo", "Yokohama"}, 2, "Tokyo is Japan's vibrant capital city."},
        {3, "Easy", 1, "Which is the longest river in the world by consensus (~6,650 km)?", {"Amazon", "Nile", "Yangtze", "Mississippi"}, 1, "The Nile River in Africa flows northward into the Mediterranean Sea."},
        {4, "Easy", 1, "Which continent contains the Amazon Rainforest?", {"Africa", "Asia", "South America", "Oceania"}, 2, "South America holds the vast Amazon Basin."},
        {5, "Easy", 1, "What is the highest mountain peak above sea level on Earth (8,848.86 m)?", {"K2", "Kangchenjunga", "Mount Everest", "Lhotse"}, 2, "Mount Everest in the Himalayas is Earth's highest peak."},
        {6, "Medium", 2, "Which river borders or flows through the most sovereign countries (10)?", {"The Rhine", "The Danube", "The Volga", "The Congo"}, 1, "The Danube touches 10 European countries on its way to the Black Sea."},
        {7, "Medium", 2, "Lake Baikal in Siberia contains what fraction of global unfrozen freshwater?", {"5%", "10%", "20%", "35%"}, 2, "Lake Baikal holds ~20% of the world's liquid surface freshwater."},
        {8, "Hard", 3, "Which country has over 200 Nubian pyramids, outnumbering Egypt?", {"Sudan", "Libya", "Chad", "Algeria"}, 0, "Sudan has over 200 pyramids built by the ancient Kingdom of Kush."},
        {9, "Hard", 4, "Which body of water in the North Atlantic is bounded entirely by ocean currents?", {"Baltic Sea", "Caspian Sea", "Sargasso Sea", "Ross Sea"}, 2, "The Sargasso Sea has no land shores; it is defined by four ocean currents."},
        {10, "Expert", 5, "Counting overseas territories, which country spans the most time zones (12)?", {"Russia", "USA", "UK", "France"}, 3, "France spans 12 time zones across its global overseas territories."}
    };
}

std::vector<Question> sportsQuestions() {
    return {
        {1, "Easy", 1, "How many players are on the field for one football (soccer) team?", {"9", "10", "11", "12"}, 2, "A standard football match fields 11 players per team."},
        {2, "Easy", 1, "In which sport do teams compete for the prestigious Stanley Cup?", {"Baseball", "Basketball", "Ice Hockey", "American Football"}, 2, "The Stanley Cup is the championship trophy of the NHL in ice hockey."},
        {3, "Easy", 1, "How many points is a touchdown worth in American football?", {"3", "4", "6", "7"}, 2, "A touchdown scores 6 points before conversion attempts."},
        {4, "Easy", 1, "A triathlon consists of which three consecutive endurance sports?", {"Rowing, Cycling, Run", "Swimming, Cycling, Running", "Ski, Shoot, Run", "Paddle, Bike, Hike"}, 1, "Triathletes complete swimming, cycling, and running."},
        {5, "Easy", 1, "How many holes are played in a standard full round of golf?", {"9", "14", "18", "21"}, 2, "A standard golf round consists of 18 holes."},
        {6, "Medium", 2, "Which nation has won the men's FIFA World Cup the most times (5)?", {"Germany", "Italy", "Argentina", "Brazil"}, 3, "Brazil won 5 FIFA World Cup championships (1958, 62, 70, 94, 2002)."},
        {7, "Medium", 2, "Which tennis Grand Slam is contested exclusively on natural grass courts?", {"Australian Open", "Roland Garros", "Wimbledon", "US Open"}, 2, "The Championships at Wimbledon is played on pristine grass courts in London."},
        {8, "Hard", 3, "What is the maximum standard break score achievable in professional snooker?", {"140", "147", "155", "160"}, 1, "A maximum 147 break pots 15 reds with 15 blacks and all six colours."},
        {9, "Hard", 4, "What ~192m foot race was the sole athletic event in the earliest ancient Olympics?", {"Diaulos", "Dolichos", "The Stadion", "Hoplitodromos"}, 2, "The Stadion foot sprint was the sole Olympic event from 776 BC to 724 BC."},
        {10, "Expert", 5, "Who won individual gold in both 100m and 200m at three straight Olympic Games?", {"Carl Lewis", "Usain Bolt", "Michael Johnson", "Jesse Owens"}, 1, "Usain Bolt captured 100m and 200m gold at Beijing 2008, London 2012, Rio 2016."}
    };
}

std::vector<Question> literatureQuestions() {
    return {
        {1, "Easy", 1, "Who authored 'The Hobbit' and 'The Lord of the Rings' epic fantasy?", {"C.S. Lewis", "J.R.R. Tolkien", "George R.R. Martin", "Conan Doyle"}, 1, "J.R.R. Tolkien was an Oxford philologist who created Middle-earth."},
        {2, "Easy", 1, "In Shakespeare's tragedy, Hamlet is the prince of which royal realm?", {"Norway", "Scotland", "Denmark", "England"}, 2, "Prince Hamlet is the tragic protagonist of Elsinore Castle in Denmark."},
        {3, "Easy", 1, "Which author created Sherlock Holmes and Dr. John H. Watson?", {"Agatha Christie", "Sir Arthur Conan Doyle", "Edgar Allan Poe", "H.G. Wells"}, 1, "Conan Doyle debuted Holmes in 1887's 'A Study in Scarlet'."},
        {4, "Easy", 1, "What is the title of George Orwell's 1949 dystopian warning against Big Brother?", {"Animal Farm", "Brave New World", "1984", "Fahrenheit 451"}, 2, "1984 introduced concepts of Newspeak, Thought Police, and Big Brother."},
        {5, "Easy", 1, "Who wrote the 1818 gothic masterpiece 'Frankenstein'?", {"Jane Austen", "Emily Bronte", "Mary Shelley", "Virginia Woolf"}, 2, "Mary Shelley conceived Frankenstein at age 18 during the year without summer."},
        {6, "Medium", 2, "In Homer's 'Odyssey', what is the name of Odysseus's faithful wife on Ithaca?", {"Helen", "Penelope", "Clytemnestra", "Andromache"}, 1, "Penelope cleverly outwitted 108 suitors while awaiting Odysseus's return."},
        {7, "Medium", 2, "Which novel opens: 'All happy families are alike; each unhappy family is unhappy in its own way'?", {"War and Peace", "Crime and Punishment", "Anna Karenina", "Brothers Karamazov"}, 2, "Leo Tolstoy's Anna Karenina begins with this celebrated opening statement."},
        {8, "Hard", 3, "Which French master wrote the 7-volume 'In Search of Lost Time' (A la recherche du temps perdu)?", {"Flaubert", "Marcel Proust", "Camus", "Zola"}, 1, "Marcel Proust's epic explored memory, highlighted by the madeleine cake episode."},
        {9, "Hard", 4, "In Dante's 'Divine Comedy', which Roman poet guides Dante through Inferno?", {"Ovid", "Horace", "Virgil", "Cicero"}, 2, "Virgil, author of the Aeneid, guides Dante through Hell and Purgatory."},
        {10, "Expert", 5, "Which 11th-century Japanese work by Murasaki Shikibu is considered the world's first novel?", {"Pillow Book", "Heike", "The Tale of Genji (源氏物語)", "Kokoro"}, 2, "The Tale of Genji was composed in Heian Japan and is the world's first psychological novel."}
    };
}

std::vector<Question> generalQuestions() {
    return {
        {1, "Easy", 1, "What is the largest living animal species on Earth?", {"African Elephant", "Blue Whale", "Colossal Squid", "Giraffe"}, 1, "The Blue Whale can reach 30m in length and weigh up to 190 metric tons."},
        {2, "Easy", 1, "What currency is officially used across Japan?", {"Yuan", "Won", "Yen", "Baht"}, 2, "The Japanese Yen (Yen / JPY) is Japan's currency."},
        {3, "Easy", 1, "How many days are in a standard Gregorian leap year?", {"364", "365", "366", "367"}, 2, "A leap year has 366 days, adding February 29th."},
        {4, "Easy", 1, "Which color is formed when blue and yellow pigments are mixed?", {"Purple", "Green", "Orange", "Brown"}, 1, "Subtractive color mixing of blue and yellow yields green."},
        {5, "Easy", 1, "What is the traditional Japanese art of paper folding called?", {"Ikebana", "Origami", "Bonsai", "Ukiyo-e"}, 1, "Origami transforms sheets of paper into sculptures without scissors."},
        {6, "Medium", 2, "Which bird is celebrated as the national bird of Japan (Kiji)?", {"Red-crowned Crane", "Japanese Robin", "Green Pheasant", "Barn Swallow"}, 2, "The Green Pheasant (Phasianus versicolor / Kiji) is Japan's national bird."},
        {7, "Medium", 2, "What is the international calling country code for Japan?", {"+44", "+49", "+81", "+82"}, 2, "+81 is Japan's international telecommunication prefix."},
        {8, "Hard", 3, "In the NATO phonetic spelling alphabet, what word represents 'M'?", {"Mango", "Mike", "Madrid", "Metro"}, 1, "The NATO alphabet uses 'Mike' for the letter M."},
        {9, "Hard", 4, "What is the traditional vermilion gate marking a sacred Shinto shrine called?", {"Torii (鳥居)", "Shoji", "Tatami", "Pagoda"}, 0, "A Torii gate demarcates sacred shrine grounds from the secular world."},
        {10, "Expert", 5, "Which Japanese philosophy finds beauty in imperfection and impermanence?", {"Yugen", "Mono no aware", "Wabi-Sabi (侘寂)", "Kintsugi"}, 2, "Wabi-Sabi honors simplicity, rustic elegance, and the natural cycle of aging."}
    };
}

std::vector<Question> logicQuestions() {
    return {
        {1, "Easy", 1, "What is the next number in the sequence: 2, 4, 8, 16, 32, ...?", {"48", "60", "64", "128"}, 2, "Each term doubles: 32 x 2 = 64."},
        {2, "Easy", 1, "If all roses are flowers and some flowers fade quickly, what must be true?", {"All roses fade", "No roses fade", "Some roses may fade quickly", "All flowers are roses"}, 2, "It is logically possible that some roses belong to the quick-fading flower group."},
        {3, "Easy", 1, "If yesterday was Tuesday, what day will it be 3 days after tomorrow?", {"Sunday", "Monday", "Tuesday", "Wednesday"}, 1, "Yesterday = Tue => Today = Wed => Tomorrow = Thu => +3 days = Monday."},
        {4, "Easy", 1, "A bat and ball cost $1.10. The bat costs $1.00 more than the ball. Ball price?", {"$0.10", "$0.05", "$0.01", "$0.15"}, 1, "2x + 1.00 = 1.10 => 2x = 0.10 => x = $0.05 (5 cents)."},
        {5, "Easy", 1, "Complete the analogy: Ocean is to Water as Forest is to:", {"Sand", "Trees", "Clouds", "Rivers"}, 1, "Trees are the constituent living element of a forest."},
        {6, "Medium", 2, "If 5 machines make 5 widgets in 5 minutes, how long do 100 machines take to make 100 widgets?", {"1 min", "5 mins", "20 mins", "100 mins"}, 1, "Each machine takes 5 minutes to produce 1 widget; 100 in parallel take 5 minutes."},
        {7, "Medium", 2, "You enter a dark cabin with ONE match. What must you light first?", {"Candle", "Oil lamp", "The match", "Wood stove"}, 2, "You must light the match before you can light anything else!"},
        {8, "Hard", 3, "3 mislabeled fruit boxes: Apples, Oranges, Both. Which box should you inspect?", {"Apples", "Oranges", "Apples & Oranges", "Any box"}, 2, "Drawing from 'Both' reveals its true content, allowing deduction of all three."},
        {9, "Hard", 4, "With 23 people, what is the probability that at least two share a birthday?", {"~10%", "~25%", "~50.7%", "~75%"}, 2, "By the Birthday Paradox, 23 people give 253 pairs, yielding ~50.7% probability."},
        {10, "Expert", 5, "In the Monty Hall problem (3 doors), what is your win probability if you SWITCH?", {"1/3 (33%)", "1/2 (50%)", "2/3 (66.7%)", "3/4 (75%)"}, 2, "The original door has a 1/3 chance; the remaining unpicked door carries 2/3."}
    };
}

// ===== Category List =====

std::vector<QuizCategory> getAllCategories() {
    return {
        {"Science",           "科学 (Kagaku)",    "[*]", SAKURA_ROSE,   scienceQuestions},
        {"Technology",        "技術 (Gijutsu)",   "[T]", CYN,           techQuestions},
        {"Mathematics",       "数学 (Sugaku)",    "[M]", SAKURA_DARK,   mathQuestions},
        {"History",           "歴史 (Rekishi)",   "[H]", LANTERN_GOLD,  historyQuestions},
        {"Geography",         "地理 (Chiri)",     "[G]", MYSTIC_PURPLE, geographyQuestions},
        {"Sports",            "競技 (Kyogi)",     "[S]", GRN,           sportsQuestions},
        {"Literature",        "文学 (Bungaku)",   "[L]", SAKURA_PALE,   literatureQuestions},
        {"General Knowledge", "常識 (Joshiki)",   "[K]", NIGHT_INDIGO,  generalQuestions},
        {"Logical Reasoning", "論理 (Ronri)",     "[R]", YEL,           logicQuestions}
    };
}

// ===== File-Backed Score Storage & Tie-Breaking Leaderboard =====

const std::string SCORES_FILE = "nexus_scores.dat";

void saveScoreRecord(const ScoreRecord& sr) {
    std::ofstream f(SCORES_FILE, std::ios::app);
    if (!f) return;
    f << sr.playerName << "|" << sr.score << "|" << sr.maxScore
      << "|" << sr.category << "|" << sr.date
      << "|" << std::fixed << std::setprecision(1) << sr.completionTime << "\n";
}

std::vector<ScoreRecord> loadScoreRecords() {
    std::vector<ScoreRecord> records;
    std::ifstream f(SCORES_FILE);
    if (!f) return records;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string token;
        ScoreRecord sr;
        if (!std::getline(ss, token, '|')) continue; sr.playerName = token;
        if (!std::getline(ss, token, '|')) continue; sr.score = std::stoi(token);
        if (!std::getline(ss, token, '|')) continue; sr.maxScore = std::stoi(token);
        if (!std::getline(ss, token, '|')) continue; sr.category = token;
        if (!std::getline(ss, token, '|')) continue; sr.date = token;
        if (std::getline(ss, token, '|')) {
            try { sr.completionTime = std::stod(token); } catch (...) { sr.completionTime = 0.0; }
        } else {
            sr.completionTime = 0.0;
        }
        records.push_back(sr);
    }
    return records;
}

// ===== Real-Time Leaderboard Display =====

void displayLeaderboard() {
    clearScreen();
    std::cout << "\n";
    printSakuraBar(74, "*~*", SAKURA_ROSE);
    printCentered("SAKURA HALL OF CHAMPIONS - REAL-TIME LEADERBOARD", 74, LANTERN_GOLD);
    printCentered("Ranked by: 1. Total Marks (Max 20) | 2. Fastest Time Tie-Breaker", 74, SAKURA_PALE);
    printSakuraBar(74, "*~*", SAKURA_ROSE);
    std::cout << "\n";

    auto scores = loadScoreRecords();
    if (scores.empty()) {
        printCentered("No champion records found yet. Begin the Sakura trial!", 74, DIM);
        std::cout << "\n";
        return;
    }

    // Ranking priority:
    // 1. Highest total marks first (b.score > a.score)
    // 2. Faster completion time ranks higher (a.time < b.time)
    std::sort(scores.begin(), scores.end(), [](const ScoreRecord& a, const ScoreRecord& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.completionTime < b.completionTime;
    });

    // Assign identical ranks if score and time are identical
    int rank = 1;
    for (size_t i = 0; i < scores.size(); ++i) {
        if (i > 0 && scores[i].score == scores[i-1].score &&
            std::abs(scores[i].completionTime - scores[i-1].completionTime) < 0.1) {
            scores[i].rank = scores[i-1].rank;
        } else {
            scores[i].rank = rank;
        }
        ++rank;
    }

    std::cout << BOLD << SAKURA_ROSE
              << "  " << std::left
              << std::setw(6)  << "Rank"
              << std::setw(18) << "Player"
              << std::setw(20) << "Arena / Category"
              << std::setw(12) << "Marks"
              << std::setw(10) << "Time"
              << "Date" << RST << "\n";
    printSakuraBar(74, "-", SAKURA_DARK);

    int count = 0;
    for (const auto& sr : scores) {
        if (++count > 25) break; // Display top 25

        std::string rankStr;
        std::string rankColor;
        if (sr.rank == 1) { rankStr = " [1st]"; rankColor = LANTERN_GOLD; }
        else if (sr.rank == 2) { rankStr = " [2nd]"; rankColor = BWHT; }
        else if (sr.rank == 3) { rankStr = " [3rd]"; rankColor = SAKURA_ROSE; }
        else { rankStr = " #" + std::to_string(sr.rank); rankColor = SAKURA_PALE; }

        std::string marksStr = std::to_string(sr.score) + "/20";
        std::string timeStr = (sr.completionTime > 0) ? (std::to_string((int)sr.completionTime) + "s") : "--";
        std::string catName = sr.category.size() > 18 ? sr.category.substr(0, 17) + "." : sr.category;
        std::string name = sr.playerName.size() > 16 ? sr.playerName.substr(0, 15) + "." : sr.playerName;

        std::cout << rankColor << "  " << std::left << std::setw(6) << rankStr << RST
                  << BWHT          << std::setw(18) << name << RST
                  << SAKURA_PALE   << std::setw(20) << catName << RST
                  << LANTERN_GOLD  << std::setw(12) << marksStr << RST
                  << CYN           << std::setw(10) << timeStr << RST
                  << DIM << sr.date << RST << "\n";
    }

    printSakuraBar(74, "-", SAKURA_DARK);
    std::cout << "\n";
}

// ===== Quiz Execution Engine (10 Questions / 180s Countdown) =====

void runQuizSession(const std::string& playerName, const QuizCategory& cat) {
    auto questions = cat.loader();
    int totalQuestions = (int)questions.size();
    if (totalQuestions > 10) totalQuestions = 10;

    int totalMarks = 0;
    int correctCount = 0;
    int incorrectCount = 0;
    std::vector<AnswerRecord> records;

    const char optLetters[] = {'A', 'B', 'C', 'D'};
    const int GLOBAL_TIMER_SECONDS = 180; // 3 Minutes

    clearScreen();
    std::cout << "\n";
    printSakuraBar(74, "*~*", SAKURA_ROSE);
    printCentered("ENTERING SAKURA ARENA: " + cat.name + " (" + cat.japaneseName + ")", 74, BWHT);
    printCentered("Rules: 10 Questions | 180 Seconds Total | Maximum 20 Marks", 74, LANTERN_GOLD);
    printSakuraBar(74, "*~*", SAKURA_ROSE);
    std::cout << "\n";

    typeWrite("  Readying trial questions...", 20, SAKURA_PALE);
    sakuraFallingEffect(4);

    auto startTime = std::chrono::steady_clock::now();

    for (int qi = 0; qi < totalQuestions; ++qi) {
        clearScreen();
        const Question& q = questions[qi];

        // Compute elapsed & remaining time
        auto now = std::chrono::steady_clock::now();
        int elapsedSec = (int)std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
        int remainingSec = GLOBAL_TIMER_SECONDS - elapsedSec;
        if (remainingSec <= 0) {
            std::cout << "\n" << SHRINE_RED << BOLD << "  [TIME EXPIRED!] 180s total duration reached!" << RST << "\n\n";
            sleepMs(1000);
            break;
        }

        std::cout << "\n";
        std::cout << cat.color << BOLD << "  " << cat.icon << " " << cat.name << RST
                  << "  |  " << SAKURA_PINK << "Marks: " << q.marks << " pt" << (q.marks > 1 ? "s" : "")
                  << " (" << q.difficulty << ")" << RST
                  << "  |  " << LANTERN_GOLD << "Score: " << totalMarks << "/20" << RST
                  << "  |  " << (remainingSec < 30 ? SHRINE_RED : CYN) << "Time Left: " << remainingSec << "s" << RST
                  << "\n\n";

        // Progress jump line
        std::cout << "  Progress: [";
        for (int p = 0; p < totalQuestions; ++p) {
            if (p < qi) std::cout << GRN << "#" << RST;
            else if (p == qi) std::cout << LANTERN_GOLD << ">" << RST;
            else std::cout << DIM << "." << RST;
        }
        std::cout << "] Question " << (qi + 1) << " of " << totalQuestions << "\n";

        printSakuraBar(74, "-", SAKURA_DARK);
        std::cout << "\n  " << LANTERN_GOLD << BOLD << "Q" << (qi + 1) << ". " << RST
                  << BWHT << q.text << RST << "\n\n";

        for (int oi = 0; oi < 4; ++oi) {
            std::cout << "    " << SAKURA_ROSE << BOLD << "(" << optLetters[oi] << ") " << RST
                      << BWHT << q.options[oi] << RST << "\n";
        }
        std::cout << "\n";
        printSakuraBar(74, "-", DIM);

        std::cout << "\n  " << SAKURA_PINK << BOLD << "  Your Answer (A / B / C / D): " << RST;
        std::string choice;
        std::cin >> choice;

        char c = (choice.empty()) ? 'X' : (char)toupper((unsigned char)choice[0]);
        int chosenIdx = c - 'A';

        // Check answer
        bool isCorrect = (chosenIdx == q.correctIndex);
        AnswerRecord rec;
        rec.questionIndex = qi;
        rec.chosenIndex = chosenIdx;
        rec.correctIndex = q.correctIndex;
        rec.isCorrect = isCorrect;

        if (isCorrect) {
            rec.marksAwarded = q.marks;
            totalMarks += q.marks;
            correctCount++;
        } else {
            rec.marksAwarded = 0;
            incorrectCount++;
        }
        records.push_back(rec);

        // Immediate feedback or silent classroom mode
        // Per requirement 5: "Do not reveal the correct answer while competition is in progress"
        std::cout << "\n  " << SAKURA_PALE << "[Answer recorded.] Moving to next challenge..." << RST << "\n";
        sleepMs(400);
    }

    auto endTime = std::chrono::steady_clock::now();
    double completionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count() / 1000.0;
    if (completionTime > GLOBAL_TIMER_SECONDS) completionTime = GLOBAL_TIMER_SECONDS;

    // ===== Final Results Screen =====
    clearScreen();
    double percentage = (totalMarks / 20.0) * 100.0;

    std::string grade, gradeColor, praise;
    if (totalMarks >= 18) { grade = "S"; gradeColor = LANTERN_GOLD; praise = "SAKURA GRANDMASTER - LEGENDARY!"; }
    else if (totalMarks >= 15) { grade = "A"; gradeColor = GRN; praise = "OUTSTANDING MASTERY!"; }
    else if (totalMarks >= 11) { grade = "B"; gradeColor = CYN; praise = "HONORABLE SCHOLAR!"; }
    else if (totalMarks >= 8)  { grade = "C"; gradeColor = YEL; praise = "DISCIPLINE & CONTINUED STUDY!"; }
    else                       { grade = "D"; gradeColor = RED; praise = "THE WAY OF LEARNING HAS JUST BEGUN!"; }

    std::cout << "\n\n";
    printSakuraBar(74, "*~*", gradeColor);
    printCentered("T R I A L   C O M P L E T E   -   S A K U R A   E D I T I O N", 74, gradeColor);
    printSakuraBar(74, "*~*", gradeColor);
    std::cout << "\n";

    printCentered("+-----------------------------------+", 74, gradeColor);
    printCentered("|      FINAL GRADE : [ " + grade + " ]          |", 74, gradeColor);
    printCentered("+-----------------------------------+", 74, gradeColor);
    std::cout << "\n";
    printCentered(praise, 74, gradeColor);
    std::cout << "\n";

    std::cout << "  " << SAKURA_ROSE << BOLD << "  Player Name      : " << RST << BWHT << playerName << "\n"
              << "  " << SAKURA_ROSE << BOLD << "  Arena / Topic    : " << RST << cat.color << cat.name << RST << "\n"
              << "  " << SAKURA_ROSE << BOLD << "  Total Marks      : " << RST << LANTERN_GOLD << BOLD << totalMarks << " / 20" << RST << "\n"
              << "  " << SAKURA_ROSE << BOLD << "  Correct Answers  : " << RST << GRN << correctCount << " / " << totalQuestions << RST << "\n"
              << "  " << SAKURA_ROSE << BOLD << "  Incorrect Answers: " << RST << RED << incorrectCount << RST << "\n"
              << "  " << SAKURA_ROSE << BOLD << "  Completion Time  : " << RST << CYN << std::fixed << std::setprecision(1) << completionTime << " seconds" << RST << "\n"
              << "  " << SAKURA_ROSE << BOLD << "  Percentage       : " << RST << gradeColor << std::fixed << std::setprecision(1) << percentage << "%" << RST << "\n\n";

    printSakuraBar(74, "-", DIM);
    std::cout << "  " << SAKURA_PALE << BOLD << "Review & Insights (Now Revealed):" << RST << "\n\n";
    for (size_t i = 0; i < records.size(); ++i) {
        const auto& r = records[i];
        const auto& q = questions[r.questionIndex];
        std::cout << "  Q" << (i + 1) << " [" << (r.isCorrect ? (std::string(GRN) + "CORRECT" + RST) : (std::string(RED) + "WRONG" + RST)) << "] "
                  << SAKURA_PALE << "(" << q.marks << " pt" << (q.marks > 1 ? "s" : "") << "): " << RST
                  << BWHT << q.text << RST << "\n";
        std::cout << "     Answer: " << SAKURA_ROSE << optLetters[q.correctIndex] << ") " << q.options[q.correctIndex] << RST << "\n";
        std::cout << "     Insight: " << DIM << q.explanation << RST << "\n\n";
    }

    // Save score record
    ScoreRecord sr;
    sr.playerName = playerName;
    sr.category = cat.name;
    sr.score = totalMarks;
    sr.maxScore = 20;
    sr.completionTime = completionTime;
    sr.correctCount = correctCount;
    sr.incorrectCount = incorrectCount;
    sr.unansweredCount = totalQuestions - (int)records.size();
    sr.date = currentDate();

    saveScoreRecord(sr);
    std::cout << "  " << SAKURA_ROSE << "[OK] Score saved to Sakura records & nexus_scores.dat!" << RST << "\n\n";

    std::cout << "  " << DIM << "Press Enter to return to main sanctuary..." << RST;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// ===== Main Menu =====

void mainMenu(const std::string& playerName) {
    auto categories = getAllCategories();
    while (true) {
        clearScreen();
        std::cout << "\n";
        printSakuraBar(74, "*~*", SAKURA_ROSE);
        printCentered("S A K U R A   Q U I Z   A R E N A   -   M A I N   S H R I N E", 74, SAKURA_PALE);
        printCentered("Welcome, Sensei / Scholar " + playerName + "!", 74, BWHT);
        printSakuraBar(74, "*~*", SAKURA_ROSE);
        std::cout << "\n";

        printCentered("SELECT A KNOWLEDGE ARENA (10 Questions | 180s | Max 20 Marks):", 74, LANTERN_GOLD);
        std::cout << "\n";

        for (int i = 0; i < (int)categories.size(); ++i) {
            std::cout << "  " << categories[i].color << BOLD
                      << "  [" << (i + 1) << "] " << categories[i].icon << " "
                      << std::left << std::setw(24) << categories[i].name
                      << DIM << categories[i].japaneseName << RST << "\n";
        }

        std::cout << "\n";
        printSakuraBar(74, "-", DIM);
        std::cout << "  " << LANTERN_GOLD << "  [L] " << RST << BWHT << "Hall of Champions (Real-Time Leaderboard)\n";
        std::cout << "  " << CYN          << "  [W] " << RST << BWHT << "Launch Multiplayer Web Server (Classroom Host)\n";
        std::cout << "  " << SHRINE_RED    << "  [Q] " << RST << DIM  << "Exit Sakura Realm\n\n";
        printSakuraBar(74, "-", DIM);
        std::cout << "\n  " << SAKURA_PINK << BOLD << "  Enter your choice: " << RST;

        std::string choice;
        std::cin >> choice;
        if (choice.empty()) continue;
        char c = (char)toupper((unsigned char)choice[0]);

        if (c == 'Q') {
            clearScreen();
            std::cout << "\n\n";
            printCentered("May the cherry blossoms light your path of wisdom.", 74, SAKURA_PINK);
            printCentered("Sayonara! (さようなら)", 74, SAKURA_PALE);
            std::cout << "\n\n";
            break;
        } else if (c == 'L') {
            displayLeaderboard();
            std::cout << "  " << DIM << "Press Enter to return..." << RST;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
        } else if (c == 'W') {
            clearScreen();
            std::cout << "\n  " << SAKURA_ROSE << BOLD << "Launching Sakura Multiplayer Web Server..." << RST << "\n";
            std::cout << "  " << SAKURA_PALE << "You can run start_server.bat to host for the classroom." << RST << "\n\n";
            std::cout << "  " << DIM << "Press Enter to return..." << RST;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
        } else if (c >= '1' && c <= '9') {
            int idx = c - '1';
            runQuizSession(playerName, categories[idx]);
        }
    }
}

// ===== Entry Point =====

int main() {
    enableVT();
    srand((unsigned)time(nullptr));

    splashScreen();

    clearScreen();
    std::cout << "\n\n";
    printSakuraBar(74, "*~*", SAKURA_ROSE);
    printCentered("S A K U R A   E N T R Y   R E G I S T R A T I O N", 74, SAKURA_PALE);
    printSakuraBar(74, "*~*", SAKURA_ROSE);
    std::cout << "\n  " << SAKURA_PINK << BOLD << "  Enter your Scholar Name (Codename): " << RST;

    std::string name;
    std::cin >> name;
    if (name.empty()) name = "SakuraScholar";

    mainMenu(name);
    return 0;
}
