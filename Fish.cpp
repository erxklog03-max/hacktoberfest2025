#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <chrono>
#include <thread>
#include <algorithm>
#include <limits>

// --- Generator Angka Acak ---
std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

// --- Struktur Data ---

struct Fish {
    std::string name;
    std::string rarity; // Common, Uncommon, Rare, Epic, Mythic
    int minSize, maxSize;
    int valuePerSize;
    int difficulty; // 1-10, mempengaruhi mini-game reeling
};

struct Player {
    int level = 1, xp = 0, xpToNext = 100;
    long long coins = 50;
    std::string currentRod = "Pancingan Awal";
    std::string currentReel = "Penggulung Awal";
    std::string currentLine = "Senar Awal";
    std::vector<std::pair<std::string, int>> fishInventory;
    std::map<std::string, int> fishOPedia; // Fish name -> count
};

struct Island {
    std::string name;
    long long cost;
    bool unlocked;
    std::map<std::string, int> fishChances; // Rarity -> chance percentage
};

// --- Database Game ---
Player player;
std::map<std::string, Fish> fishDB;
std::map<std::string, Island> islandDB;
std::map<std::string, int> rodDB, reelDB, lineDB; // Gear name -> power/price

// --- Fungsi Bantu ---
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}
void separator() { std::cout << "\n--------------------------------------------------\n"; }
void shortPause(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
void pause() {
    std::cout << "\n(Tekan Enter untuk melanjutkan...)";
    // Membersihkan sisa buffer input
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// --- Inisialisasi Game ---
void initializeGame() {
    // Ikan
    fishDB["Teri"] = {"Teri", "Common", 5, 15, 1, 1};
    fishDB["Sarden"] = {"Sarden", "Common", 10, 25, 1, 2};
    fishDB["Makarel"] = {"Uncommon", "Uncommon", 20, 40, 2, 3};
    fishDB["Ikan Badut"] = {"Uncommon", "Uncommon", 15, 30, 3, 3};
    fishDB["Kakap Merah"] = {"Rare", "Rare", 50, 100, 5, 5};
    fishDB["Barakuda"] = {"Rare", "Rare", 80, 150, 6, 6};
    fishDB["Hiu Martil"] = {"Epic", "Epic", 200, 300, 15, 8};
    fishDB["Megalodon"] = {"Mythic", "Mythic", 500, 1000, 50, 10};

    // Pulau
    islandDB["Dermaga Tenang"] = {"Dermaga Tenang", 0, true, {{"Common", 70}, {"Uncommon", 30}}};
    islandDB["Pulau Karang"] = {"Pulau Karang", 2500, false, {{"Common", 20}, {"Uncommon", 50}, {"Rare", 30}}};
    islandDB["Laut Dalam"] = {"Laut Dalam", 20000, false, {{"Rare", 40}, {"Epic", 50}, {"Mythic", 10}}};

    // Peralatan
    rodDB = {{"Pancingan Awal", 10}, {"Pancingan Fiber", 500}, {"Pancingan Karbon", 10000}};
    reelDB = {{"Penggulung Awal", 10}, {"Penggulung Cepat", 750}, {"Penggulung Pro", 12000}};
    lineDB = {{"Senar Awal", 10}, {"Senar Kuat", 600}, {"Senar Baja", 11000}};
}

// --- Mekanik Mini-Game ---

bool reelingMinigame(const Fish& fish) {
    int fishPos = 5;
    const int barLength = 40;
    const int safeZoneStart = 15;
    const int safeZoneEnd = 25;
    int reelProgress = 0;
    const int reelTarget = 100;

    int reelPower = reelDB[player.currentReel] / 10;
    int lineStrength = lineDB[player.currentLine];

    std::cout << "Tahan ikan di zona aman untuk menariknya!\n";
    shortPause(1500);

    while (reelProgress < reelTarget) {
        clearScreen();
        std::cout << "Menarik " << fish.name << "!\n";
        
        // Pergerakan ikan (acak dan berdasarkan difficulty)
        int fishMovement = (rng() % (fish.difficulty + 1)) - (rng() % 3);
        fishPos -= fishMovement;
        fishPos = std::max(0, std::min(barLength, fishPos));

        // Tampilkan bar
        std::cout << "[";
        for (int i = 0; i < barLength; ++i) {
            if (i == fishPos) std::cout << "F";
            else if (i >= safeZoneStart && i <= safeZoneEnd) std::cout << "=";
            else std::cout << "-";
        }
        std::cout << "]\n";
        std::cout << "Progres: " << reelProgress << "% | Tekan [enter] untuk menarik, ketik 'lepas' untuk menyerah.\n> ";

        // Input pemain dengan timeout
        std::thread inputThread;
        std::string userInput = "";
        bool receivedInput = false;
        
        inputThread = std::thread([&](){
            std::getline(std::cin, userInput);
            receivedInput = true;
        });
        
        // Timeout 1 detik
        shortPause(1000);
        
        if (receivedInput) {
            inputThread.detach(); // Hentikan thread jika sudah dapat input
            if(userInput == "lepas") return false;
            fishPos += (2 + reelPower); // Pemain menarik ikan
        } else {
             inputThread.detach(); // Hentikan thread jika timeout
        }


        // Update progres
        if (fishPos >= safeZoneStart && fishPos <= safeZoneEnd) {
            reelProgress += (5 + reelPower);
        } else {
            reelProgress -= 5;
            if (reelProgress < 0) reelProgress = 0;
            // Hukuman jika ikan terlalu jauh
            if (fishPos < 5 || fishPos > barLength - 5) {
                lineStrength -= fish.difficulty;
                std::cout << "Senar menegang!\n";
                if (lineStrength <= 0) {
                    std::cout << "PUTUS! Ikannya lepas!\n";
                    pause();
                    return false;
                }
            }
        }
    }
    return true;
}

void fishingMinigame(const Island& island) {
    clearScreen();
    // 1. Casting
    std::cout << "Tekan [enter] untuk menghentikan power bar!\n";
    std::cout << "[";
    int power = 0;
    for (int i = 0; i <= 100; i += 5) {
        std::string bar = "";
        for (int j = 0; j < i / 5; ++j) bar += "=";
        std::cout << "\r[" << bar << std::string(20 - bar.length(), ' ') << "]" << std::flush;
        shortPause(50);
        // Cek jika ada input tanpa blocking
        // Ini adalah cara sederhana, di console murni sulit untuk non-blocking
    }
    // Untuk simplifikasi, kita buat casting otomatis
    std::cout << "\nKail dilempar jauh!\n";
    shortPause(2000);

    // 2. Hooking
    std::cout << "Tunggu sambaran...\n";
    shortPause(std::uniform_int_distribution<int>(2000, 7000)(rng));
    std::cout << "\n!!! FISH ON !!!\nTEKAN [ENTER] SEKARANG!\n";
    auto hookStart = std::chrono::high_resolution_clock::now();
    std::cin.get();
    auto hookEnd = std::chrono::high_resolution_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(hookEnd - hookStart).count() > 1500) {
        std::cout << "Terlalu lambat! Ikan kabur!\n";
        pause();
        return;
    }
    std::cout << "Kena!\n";

    // Pilih ikan berdasarkan kelangkaan
    int roll = std::uniform_int_distribution<int>(1, 100)(rng);
    std::string rarity;
    int cumulative = 0;
    if (island.fishChances.count("Mythic") && roll <= (cumulative += island.fishChances.at("Mythic"))) rarity = "Mythic";
    else if (island.fishChances.count("Epic") && roll <= (cumulative += island.fishChances.at("Epic"))) rarity = "Epic";
    else if (island.fishChances.count("Rare") && roll <= (cumulative += island.fishChances.at("Rare"))) rarity = "Rare";
    else if (island.fishChances.count("Uncommon") && roll <= (cumulative += island.fishChances.at("Uncommon"))) rarity = "Uncommon";
    else rarity = "Common";
    
    std::vector<std::string> possibleFish;
    for(const auto& pair : fishDB) {
        if(pair.second.rarity == rarity) possibleFish.push_back(pair.first);
    }
    
    Fish caughtFish = fishDB[possibleFish[std::uniform_int_distribution<int>(0, possibleFish.size() - 1)(rng)]];

    // 3. Reeling
    if (reelingMinigame(caughtFish)) {
        int size = std::uniform_int_distribution<int>(caughtFish.minSize, caughtFish.maxSize)(rng);
        long long value = size * caughtFish.valuePerSize;
        int xp = size;

        std::cout << "\nSELAMAT! Kamu menangkap " << caughtFish.name << "!\n";
        std::cout << "Ukuran: " << size << " cm | Harga: " << value << " Koin | XP: " << xp << "\n";
        
        player.coins += value;
        player.xp += xp;
        player.fishOPedia[caughtFish.name]++;
    }
    pause();
}

// --- Menu & Navigasi ---
void mainMenu(); // Forward declaration

void chooseIsland() {
    clearScreen();
    std::cout << "Pilih Pulau Tujuan:\n";
    int i = 1;
    std::vector<std::string> islandNames;
    for(auto const& [name, island] : islandDB) {
        if(island.unlocked) {
            std::cout << "[" << i++ << "] " << name << "\n";
            islandNames.push_back(name);
        }
    }
    std::cout << "[" << i++ << "] Kembali\n> ";
    int choice;
    std::cin >> choice;
    std::cin.get(); // consume newline
    
    if (choice > 0 && choice <= islandNames.size()) {
        fishingMinigame(islandDB[islandNames[choice - 1]]);
    }
}

void visitShop() {
     clearScreen();
    std::cout << "Selamat datang! Apa yang mau kamu beli atau buka?\n";
    std::cout << "Koin-mu: " << player.coins << "\n";
    separator();
    std::cout << "[1] Buka Pulau Baru\n";
    std::cout << "[2] Upgrade Peralatan\n";
    std::cout << "[3] Kembali\n> ";
    std::string choice;
    std::getline(std::cin, choice);

    // Implementasi logika toko di sini
    if(choice == "1") {
        // Logika buka pulau
        std::cout << "Pulau mana yang mau dibuka?\n";
        for(auto& pair : islandDB) {
            if(!pair.second.unlocked) {
                std::cout << "- " << pair.first << " (" << pair.second.cost << " Koin)\n";
            }
        }
        std::cout << "> ";
        std::string islandChoice;
        std::getline(std::cin, islandChoice);
        if(islandDB.count(islandChoice) && !islandDB[islandChoice].unlocked) {
            if(player.coins >= islandDB[islandChoice].cost) {
                player.coins -= islandDB[islandChoice].cost;
                islandDB[islandChoice].unlocked = true;
                std::cout << "Selamat! " << islandChoice << " berhasil dibuka!\n";
            } else {
                std::cout << "Koin tidak cukup!\n";
            }
        }
    } else if(choice == "2") {
        // Logika upgrade gear
        // Bisa dibuat lebih detail seperti ini untuk Rod, Reel, dan Line
    }
    pause();
}

void viewFishOPedia() {
    clearScreen();
    std::cout << "--- FISH-O-PEDIA ---\n";
    if (player.fishOPedia.empty()) {
        std::cout << "Belum ada ikan yang ditangkap.\n";
    } else {
        for (const auto& pair : player.fishOPedia) {
            const Fish& fish = fishDB[pair.first];
            std::cout << "- " << fish.name << " (" << fish.rarity << ") | Tertangkap: " << pair.second << "\n";
        }
    }
    pause();
}

void mainMenu() {
    clearScreen();
    std::cout << "Level: " << player.level << " | XP: " << player.xp << "/" << player.xpToNext << " | Koin: " << player.coins << "\n";
    separator();
    std::cout << "[1] Pergi Memancing\n";
    std::cout << "[2] Kunjungi Toko\n";
    std::cout << "[3] Lihat Fish-o-pedia\n";
    std::cout << "[4] Keluar\n> ";
    std::string choice;
    std::getline(std::cin, choice);

    if (choice == "1") chooseIsland();
    else if (choice == "2") visitShop();
    else if (choice == "3") viewFishOPedia();
    else if (choice == "4") return;
    
    mainMenu(); // Loop back to menu
}


int main() {
    initializeGame();
    clearScreen();
    std::cout << "=======================================\n";
    std::cout << "||    PETUALANGAN PEMANCING PRO      ||\n";
    std::cout << "=======================================\n";
    std::cout << "Selamat datang di surga para pemancing!\n";
    pause();

    mainMenu();
    
    std::cout << "Terima kasih telah bermain!\n";
    return 0;
}
