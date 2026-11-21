// STEP 12 : Copy constructor vs copy assignment
//
// Compile:  g++ -std=c++17 -Wall -Wextra step12_copy.cpp -o step12 && ./step12

#include <cstdio>

class Sensor {
public:
    // Normal constructor: builds from VALUES
    explicit Sensor(int id) : id_{id} {
        std::printf("  normal constructor   : Sensor %d created\n", id_);
    }

    // Copy constructor: builds a NEW object from another Sensor
    //
    //   other           = the Sensor being copied (the source)
    //   const           = the source is NOT modified
    //   & (reference)   = the source is NOT copied to be passed in
    //                     (by value it would need... a copy -> infinite loop,
    //                      so the compiler forbids  Sensor(Sensor other))
    //
    // CALLED WHEN a new object is created from an existing one:
    //   Sensor b = a;         // declaration with '=' (case 2 in main)
    //   Sensor b{a};          // same thing, brace syntax
    //   Sensor b(a);          // same thing, parenthesis syntax
    //   byValue(a);           // passing BY VALUE: s is a copy (case 5)
    //   return s;             // returning by value (often optimized away
    //                         //   by the compiler: "copy elision")
    //   std::vector<Sensor> v; v.push_back(a);   // container stores a copy
    //
    // NOT CALLED:
    //   byReference(a);       // a reference IS the original (case 6)
    //   c = a;                // c already exists -> copy ASSIGNMENT (below)
    //
    // If you don't write it, the compiler generates one that copies each
    // member. Write it (or "= delete" it, like LockGuard in step 9) when
    // the class owns a resource: a pin, a lock, a buffer, a file...
    Sensor(const Sensor& other) : id_{other.id_} {
        std::printf("  copy constructor     : new Sensor, copy of %d\n", other.id_);
    }

    // Copy assignment: OVERWRITES an object that already exists
    //
    // CALLED WHEN the object on the left was created BEFORE:
    //   Sensor c{2};
    //   c = a;                // case 4 in main
    //   a = b = c;            // chained: works because it returns *this
    //
    // NOT CALLED:
    //   Sensor b = a;         // b is being CREATED -> copy CONSTRUCTOR,
    //                         //   even though there is an '='
    //
    // RULE: new object -> copy constructor
    //       existing object -> copy assignment
    Sensor& operator=(const Sensor& other) {
        std::printf("  copy assignment      : Sensor %d becomes a copy of %d\n", id_, other.id_);
        id_ = other.id_;
        return *this;            // return the object itself (allows a = b = c)
    }

    ~Sensor() { std::printf("  destructor           : Sensor %d destroyed\n", id_); }

private:
    int id_;
};

// ---------------------------------------------------------------------------
// PASSING BY VALUE vs BY REFERENCE
// ---------------------------------------------------------------------------
//
// BY VALUE:  void byValue(Sensor s)
//   - s is a NEW object, built by the COPY CONSTRUCTOR from the argument
//   - modifying s does NOT modify the original
//   - at the end of the function, s is destroyed -> DESTRUCTOR runs
//   - cost: one copy + one destruction per call (expensive for big objects)
//   C equivalent: void f(struct Sensor s)   -> the whole struct is copied
//
// BY REFERENCE:  void byReference(Sensor& s)
//   - s is ANOTHER NAME for the original: no new object, no copy, no destructor
//   - modifying s DOES modify the original
//   - passed like a pointer under the hood -> cheap
//   C equivalent: void f(struct Sensor* s) called as f(&a)
//   but: no '&' at the call, no '->' inside, and it can never be NULL
//
// BY CONST REFERENCE:  void print(const Sensor& s)
//   - the original, no copy, but READ-ONLY (s cannot be modified)
//   - the usual way to pass an object you only need to read
//   C equivalent: const struct Sensor*
//
// WHICH ONE TO USE
//   small types (int, bool, uint8_t, pointer, enum) -> by value
//   object you only read                            -> const T&
//   object the function must modify                 -> T&
//   you really want a copy                          -> by value
//
// This is why the copy constructor takes  const Sensor& other :
//   &     -> by value would need a copy -> endless loop
//   const -> it reads the source, never modifies it
//
// See cases 5 and 6 in main:
//   5) byValue(a)     -> prints "copy constructor" then "destructor"
//   6) byReference(a) -> prints nothing: no copy at all
// ---------------------------------------------------------------------------
void byValue(Sensor s)      { (void)s; }   // takes a COPY
void byReference(Sensor& s) { (void)s; }   // takes the ORIGINAL

int main() {
    std::printf("1) Sensor a{1};\n");
    Sensor a{1};

    std::printf("\n2) Sensor b = a;        (b is being created)\n");
    Sensor b = a;

    std::printf("\n3) Sensor c{2};\n");
    Sensor c{2};

    std::printf("\n4) c = a;               (c already exists)\n");
    c = a;

    std::printf("\n5) byValue(a);          (hidden copy)\n");
    byValue(a);

    std::printf("\n6) byReference(a);      (no copy)\n");
    byReference(a);

    std::printf("\n7) end of main\n");
    return 0;
}
