// Séance 1 : Classes et RAII
// 1) Constructeur, destructeur, liste d'initialisation
//
// Compiler :  g++ -std=c++17 -Wall -Wextra -o class class.cpp && ./class

#include <cstdio>
#include <cstdint>

// ---------------------------------------------------------------------------
// 1. Une classe simple : Led
// ---------------------------------------------------------------------------
class Led {
public:
    // --- Constructeur avec LISTE D'INITIALISATION ---------------------------
    // Syntaxe :  Classe(params) : membre1(val1), membre2(val2) { corps }
    //
    // Pourquoi la liste plutôt que des affectations dans le corps ?
    //  - Les membres sont INITIALISÉS directement (pas construits par défaut
    //    puis réaffectés) → plus efficace.
    //  - OBLIGATOIRE pour : membres const, références, et membres sans
    //    constructeur par défaut.
    Led(uint8_t pin, const char* name)
        : pin_(pin), name_(name), state_(false)
    {
        std::printf("[Led %s] constructeur : configure la broche %u en sortie\n",
                    name_, pin_);
    }

    // --- Constructeur par défaut qui DÉLÈGUE à un autre constructeur --------
    Led() : Led(13, "default") {}

    // --- Destructeur ---------------------------------------------------------
    // Appelé automatiquement quand l'objet sort de sa portée (fin de bloc),
    // ou lors d'un delete. Pas de paramètres, pas de retour, un seul par classe.
    ~Led()
    {
        std::printf("[Led %s] destructeur : éteint la LED et libère la broche %u\n",
                    name_, pin_);
    }

    void toggle()
    {
        state_ = !state_;
        std::printf("[Led %s] -> %s\n", name_, state_ ? "ON" : "OFF");
    }

private:
    const uint8_t pin_;   // const  → DOIT être initialisé dans la liste
    const char*   name_;
    bool          state_;
};

// ---------------------------------------------------------------------------
// 2. Piège : l'ordre d'initialisation
// ---------------------------------------------------------------------------
// Les membres sont initialisés dans l'ordre de leur DÉCLARATION dans la
// classe, PAS dans l'ordre écrit dans la liste d'initialisation.
class Buffer {
public:
    // ✅ Correct : size_ est déclaré avant capacity_, donc déjà initialisé.
    explicit Buffer(int size) : size_(size), capacity_(size_ * 2) {}

    // ❌ Si on avait déclaré capacity_ AVANT size_, alors capacity_(size_ * 2)
    //    lirait size_ non initialisé → comportement indéfini.
    //    -Wall (-Wreorder) vous avertit si l'ordre de la liste diffère.

    void print() const { std::printf("Buffer size=%d capacity=%d\n", size_, capacity_); }

private:
    int size_;
    int capacity_;
};

// ---------------------------------------------------------------------------
// 3. Membre référence + composition : ordre construction / destruction
// ---------------------------------------------------------------------------
class Uart {
public:
    explicit Uart(uint32_t baud) : baud_(baud)
    { std::printf("[Uart] init à %u bauds\n", static_cast<unsigned>(baud_)); }
    ~Uart() { std::printf("[Uart] désactivé\n"); }
private:
    uint32_t baud_;
};

class Logger {
public:
    // Une référence DOIT être liée dans la liste d'initialisation.
    // Led n'a pas besoin d'être construite par défaut : on la construit
    // directement avec ses arguments.
    explicit Logger(Uart& uart) : uart_(uart), statusLed_(5, "status")
    { std::printf("[Logger] prêt\n"); }

    ~Logger() { std::printf("[Logger] arrêt\n"); }

private:
    Uart& uart_;     // référence : non réassignable, doit être initialisée
    Led   statusLed_; // membre objet (composition)
};
// Ordre de construction : membres (dans l'ordre de déclaration) PUIS corps.
// Ordre de destruction  : corps du destructeur PUIS membres en ordre INVERSE.

// ---------------------------------------------------------------------------
int main()
{
    std::printf("=== 1. Durée de vie liée à la portée ===\n");
    {
        Led red(2, "red");
        Led def;          // constructeur par défaut (délégué)
        red.toggle();
        red.toggle();
        std::printf("-- fin du bloc --\n");
    }   // ← destructeurs appelés ici, en ordre INVERSE : def puis red

    std::printf("\n=== 2. Ordre d'initialisation ===\n");
    Buffer b(16);
    b.print();

    std::printf("\n=== 3. Composition ===\n");
    {
        Uart uart(115200);
        Logger log(uart);
        std::printf("-- fin du bloc --\n");
    }   // ← ~Logger, puis ~Led(status), puis ~Uart

    std::printf("\n=== fin du main ===\n");
    return 0;
}
