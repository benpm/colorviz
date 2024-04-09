// Mostly math utilities, also utilities to make C++ less of a headache
#pragma once

#include <variant>
#include <functional>
#include <atomic>
#include <array>
#include <tuple>
#include <any>
#include <memory>
#include <typeindex>
#include <memory_resource>
#include <type_traits>
#include <concepts>
#include <any>
#include <span>
#include <utility>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <filesystem>
#include <list>
#include <chrono>
#include <random>
#include <ranges>
#include <logging.hpp>
#include <queue>

namespace fs = std::filesystem;

// Singleton pattern wrapper
template <typename TClass> struct Singleton
{
    inline static TClass& get()
    {
        static std::unique_ptr<TClass> _instance = std::make_unique<TClass>();
        return *_instance.get();
    }
};

// Concept that enforces that T is enum
template <typename T>
concept enumerable = std::is_enum_v<T>;

// Concept that type T is integral or floating point
template <typename T>
concept numeric = std::is_integral_v<T> || std::is_floating_point_v<T>;

// Concept that enforces that size_t _hash() is defined
template <typename T>
concept c_Hashable = requires(const T& t) {
    {
        t._hash()
    } -> std::convertible_to<size_t>;
};
template <c_Hashable T> struct std::hash<T>
{
    constexpr size_t operator()(const T& obj) const { return obj._hash(); }
};

// True if number is power of 2
template <std::integral T> constexpr bool isPow2(T n)
{
    return n && !(n & (n - 1));
}

// Cantor's pairing function
template <std::unsigned_integral T> constexpr T cantor(T a, T b)
{
    return (a + b) * (a + b + T(1)) / T(2) + b;
}

// Cantor's pairing function (signed)
template <std::signed_integral T> constexpr T cantor(T a, T b)
{
    return cantor((a < 0) ? (-2 * a - 1) : (2 * a), (b < 0) ? (-2 * b - 1) : (2 * b));
}

// Floating point equality
template <std::floating_point T>
constexpr bool feq(T a, T b, T epsilon = std::numeric_limits<T>::epsilon())
{
    return std::abs(a - b) < epsilon;
}

// Sign function
template <std::signed_integral T> constexpr T sign(T val)
{
    return (T(0) < val) - (val < T(0));
}

// Sign function (unsigned, for completeness)
template <std::unsigned_integral T> constexpr T sign(T val)
{
    return T(0) < val;
}

// Positive remainder / positive modulo (aka Python's modulo operator)
template <std::signed_integral T> constexpr T rem(T a, T b)
{
    return (a % b + b) % b;
}

// Multiple of step such that (val * step >= val)
template <std::integral T> inline constexpr T ceildiv(T val, T step)
{
    return val / step + (val % step != 0);
}

// Floor integral val to nearest multiple of step
template <std::integral T> inline constexpr T floorStep(T val, T step)
{
    return (val / step) * step;
}

// Floor float val to nearest multiple of step
template <std::floating_point T> inline constexpr T floorStep(T val, T step)
{
    return std::floor(val / step) * step;
}

// Ceiling integral val to nearest multiple of step
template <std::integral T> inline constexpr T ceilStep(T val, T step)
{
    return ceildiv(val, step) * step;
}

// Ceiling float val to nearest multiple of step
template <std::floating_point T> inline constexpr T ceilStep(T val, T step)
{
    return std::ceil(val / step) * step;
}

// Unordered pair of values of the same type
template <std::totally_ordered TKey> struct UnorderedPair : public std::pair<TKey, TKey>
{
    UnorderedPair() = default;
    UnorderedPair(const TKey& a, const TKey& b) : std::pair<TKey, TKey>(a, b) {}

    const TKey& min() const { return std::min(this->first, this->second); }
    const TKey& max() const { return std::max(this->first, this->second); }

    constexpr size_t _hash() const { return cantor(this->min(), this->max()); }

    std::string _format() const { return fmt::format("({}, {})", this->first, this->second); }

    bool operator==(const UnorderedPair<TKey>& other) const
    {
        return this->_hash() == other._hash();
    }
};

// Creates a new, globally unique ID, thread-safe
uint32_t globalID();

// Creates a new, globally unique ID, thread-safe, casting to given type
template <typename TId = uint32_t>
    requires std::is_convertible_v<TId, uint32_t>
TId globalID()
{
    return static_cast<TId>(globalID());
}

// Creates a new, unique-to-type ID, thread-safe
template <typename TId>
    requires std::is_same_v<TId, uint32_t>
TId localID()
{
    static std::atomic_uint _id = 0u;
    return static_cast<TId>(_id++);
}

// Returns a unique ID for given type, thread-safe
template <typename T> constexpr uint32_t typeID()
{
    return (uint32_t)std::type_index(typeid(T)).hash_code();
}

// Returns the name of the given type given its ID (returns empty string if release mode)
std::string typeName(uint32_t id);

// Sets the name of the given type given its ID (does nothing in release mode)
void setTypeName(uint32_t id, std::string name);

// Returns the name of the given type (compiler-specific, not cross-platform)
template <typename T> std::string typeName()
{
    std::string n(typeid(T).name());
    setTypeName(typeID<T>(), n);
    return n;
}

// A map that uses types as keys
template <typename TValue> class TypeMap
{
   private:
    std::unordered_map<uint32_t, TValue> map;

   public:
    template <typename TKey> TValue& at() { return this->map.at(typeID<TKey>()); }
    template <typename TKey> const TValue& at() const { return this->map.at(typeID<TKey>()); }
    template <typename TKey> bool contains() const { return this->map.contains(typeID<TKey>()); }
    TValue& at(uint32_t key) { return this->map.at(key); }
    const TValue& at(uint32_t key) const { return this->map.at(key); }
    bool contains(uint32_t key) const { return this->map.contains(key); }
    template <typename TKey> TValue& getOrEmplace() { return this->getOrEmplace(typeID<TKey>()); }
    TValue& getOrEmplace(uint32_t key)
    {
        auto it = this->map.find(key);
        if (it == this->map.end()) {
            it = this->map.try_emplace(key).first;
        }
        return it->second;
    }
};

// A map that uses types as keys, where the value is templated with the key type
template <template <class> typename TValue> class TypeTemplateMap
{
   private:
    std::unordered_map<uint32_t, std::any> map;

   public:
    /**
     * @brief Returns a reference to the value associated with the given type, creating it if it
     * doesn't exist
     *
     * @tparam TKey The type used as the key
     * @return requires&
     */
    template <typename TKey> TValue<TKey>& getOrEmplace()
    {
        const uint32_t key = typeID<TValue<TKey>>();
        auto it = this->map.find(key);
        if (it == this->map.end()) {
            it = this->map.try_emplace(key, TValue<TKey>()).first;
        }
        return *std::any_cast<TValue<TKey>>(&it->second);
    }

    // Returns true if the given type is in the map
    template <typename TKey> bool contains() const
    {
        return this->map.contains(typeID<TValue<TKey>>());
    }

    template <typename TKey> TValue<TKey>& at(uint32_t key)
    {
        $assert(this->map.contains(key), "Type {} not found in map", typeName<TValue<TKey>>());
        return *std::any_cast<TValue<TKey>>(&this->map.at(key));
    }

    template <typename TKey> const TValue<TKey>& at(uint32_t key) const
    {
        $assert(this->map.contains(key), "Type {} not found in map", typeName<TValue<TKey>>());
        return *std::any_cast<TValue<TKey>>(&this->map.at(key));
    }

    template <typename TKey> TValue<TKey>& at() { return this->at<TKey>(typeID<TValue<TKey>>()); }

    template <typename TKey> const TValue<TKey>& at() const
    {
        return this->at<TKey>(typeID<TValue<TKey>>());
    }
};

constexpr uint32_t ID_BAD = std::numeric_limits<uint32_t>::max();

// Allows creating overloaded std::visit lambdas
template <class... Ts> struct $overload : Ts...
{
    using Ts::operator()...;
};
template <class... Ts> $overload(Ts...) -> $overload<Ts...>;

// Returns the index of the given type in the given variant
template <typename VariantType, typename T, std::size_t index = 0>
constexpr std::size_t variantIdx()
{
    static_assert(std::variant_size_v<VariantType> > index, "Type not found in variant");
    if constexpr (index == std::variant_size_v<VariantType>) {
        return index;
    } else if constexpr (std::is_same_v<std::variant_alternative_t<index, VariantType>, T>) {
        return index;
    } else {
        return variantIdx<VariantType, T, index + 1>();
    }
}

// Dynamically sized bitset
struct DynBitset
{
    std::vector<uint8_t> data;
    // Size in bits
    size_t size = 0u;

    DynBitset() = default;
    DynBitset(size_t size) : size(size) { this->resize(size); }

    inline bool operator[](size_t idx) const { return this->data[idx / 8u] & (1u << (idx % 8u)); }
    inline void set(size_t idx) { this->data[idx / 8u] |= (1u << (idx % 8u)); }
    inline void unset(size_t idx) { this->data[idx / 8u] &= ~(1u << (idx % 8u)); }
    inline void resize(size_t size)
    {
        this->data.resize((size + 7u) / 8u, 0u);
        this->size = size;
    }
    inline void clear()
    {
        this->data.clear();
        this->data.resize(this->size, 0u);
        this->size = 0u;
    }
};

// Randomly accessible range
template <std::random_access_iterator TIter> struct Range
{
    using reference_t = std::iter_reference_t<TIter>;

    TIter it[2];

    TIter begin() { return it[0]; }
    TIter end() { return it[1]; }
    auto rbegin() { return std::make_reverse_iterator(it[1]); }
    auto rend() { return std::make_reverse_iterator(it[0]); }

    Range() = default;
    Range(TIter begin, TIter end) : it{ begin, end } {}
    template <std::ranges::range R>
    Range(R& r) : Range(std::ranges::begin(r), std::ranges::end(r)){};
    template <std::ranges::range R>
    Range(const R& r) : Range(std::ranges::cbegin(r), std::ranges::cend(r)){};
    size_t size() const { return std::distance(this->it[0], this->it[1]); }
    reference_t operator[](size_t idx) const { return this->it[0][idx]; }

    /* printable */ std::string _format() const
    {
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < this->size(); i++) {
            ss << fmt::format("{}, ", this->it[0][i]);
        }
        ss << "]";
        return ss.str();
    }
};
template <std::ranges::range R> Range(R& r) -> Range<std::ranges::iterator_t<R>>;
template <std::ranges::range R>
Range(const R& r) -> Range<decltype(std::ranges::cbegin(std::declval<R&>()))>;

// Create container from range
template <template <class> typename TContainer, std::ranges::input_range TRange>
inline TContainer<std::ranges::range_value_t<TRange>> toContainer(TRange&& r)
{
    return { std::ranges::begin(r), std::ranges::end(r) };
}

// Input range concept
template <typename TRange, typename TValue>
concept c_InputRange = std::ranges::input_range<TRange> &&
                       std::convertible_to<std::ranges::range_value_t<TRange>, TValue>;

// Integral span (inclusive bounds)
template <std::integral T> struct Span
{
    T min = std::numeric_limits<T>::max();
    T max = std::numeric_limits<T>::min();

    // Creates a span from a min and a length
    static Span lenSpan(T min, T len) { return { min, min + len - 1 }; }

    // Default constructor
    Span() = default;
    // From values directly
    Span(T min, T max) : min(min), max(max) {}
    // From pair
    Span(const std::pair<T, T>& pair) : min(pair.first), max(pair.second) {}
    // From tuple
    Span(const std::tuple<T, T>& tuple) : min(std::get<0>(tuple)), max(std::get<1>(tuple)) {}

    // Returns size of span, zero if max < min
    inline size_t length() const { return !this->empty() ? (this->max - this->min + 1) : 0; }
    // Sets the length of the span, keeping the min
    inline void length(T len) { this->max = this->min + len - 1; }
    // Gets the length of the span if it nearest bound to origin was replace with origin
    inline size_t lengthFrom(T origin = T(0)) const { return this->maxdist(origin) + 1; }
    // Returns the maximum absolute difference between min and max and the given origin
    inline size_t maxdist(T origin = T(0)) const
    {
        return (size_t)std::max(
            std::abs((std::ptrdiff_t)(this->min - origin)),
            std::abs((std::ptrdiff_t)(this->max - origin))
        );
    }
    // Returns true if span is empty (max < min), this is true for uninit'd spans
    inline bool empty() const
    {
        return this->max < this->min || (this->min == 0 && this->max == (T)-1);
    }
    // Modifies min and max so that the span covers the given value, setting both to val if zero
    inline Span<T>& cover(T value)
    {
        if (this->empty()) {
            this->min = this->max = value;
        } else {
            this->min = std::min(this->min, value);
            this->max = std::max(this->max, value);
        }
        return *this;
    }
    inline Span<T> cover(T value) const { return Span<T>(*this).cover(value); }
    // Returns true if value is contained in the span
    inline bool contains(T value) const { return value >= this->min && value <= this->max; }
    // Returns subrange over given range that is contained in the span
    template <std::ranges::range TRange> inline auto subrange(TRange&& range) const
    {
        $assert(this->min >= 0 && this->max < std::ranges::size(range), "Span out of range");
        return std::ranges::subrange(range.begin() + this->min, range.begin() + this->max + 1);
    }

    // Returns min or max
    T operator[](size_t idx) const { return std::to_array({ this->min, this->max }).at(idx); }
    T& operator[](size_t idx) { return std::to_array({ this->min, this->max }).at(idx); }
    // Comparison operators
    bool operator==(const Span& other) const
    {
        return this->min == other.min && this->max == other.max;
    }
    // Math operators
    inline Span<T> operator+(T val) const { return { this->min + val, this->max + val }; }
    inline Span<T> operator-(T val) const { return { this->min - val, this->max - val }; }
    inline Span<T> operator*(T val) const { return { this->min * val, this->max * val }; }
    inline Span<T> operator/(T val) const { return { this->min / val, this->max / val }; }

    /// NOTE: Only min is compared for lt/gt because it makes sense for the managed buffer case
    ///       In other situations a different sorting func may be needed

    /* hashable */ size_t _hash() const { return cantor<T>(this->min, this->max); }
    /* printable */ std::string _format() const
    {
        return fmt::format("[{}, {}]", this->min, this->max);
    }
};

// Execution time timer
struct StopWatch
{
    using clock_t = std::chrono::high_resolution_clock;

    std::string label;
    clock_t::time_point timer;
    bool stopped = false;
    bool paused = false;

    // Stopwatch with given label, starts immediately if specified to
    StopWatch(std::string label, bool paused = false) : label(label), paused(paused)
    {
        if (!paused) {
            this->start();
        }
    }
    // Logs a message of the elapsed time at destruct, if stop() wasnt called
    ~StopWatch()
    {
        if (!this->stopped) {
            this->log();
        }
    }
    // Starts the timer or restarts it if it has already been started
    void start()
    {
        this->stopped = false;
        this->paused = false;
        this->restart();
    }
    // Logs a message of the elapsed time
    void log(std::string msg = "") const
    {
        $info("{} {}: {:.3f} ms", this->label, msg, (float)(this->elapsed()) / 1000.0f);
    }
    // Report microseconds elapsed since last call to start()
    size_t elapsed() const
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(clock_t::now() - this->timer)
            .count();
    }
    // Stop and log
    void stop(std::string msg = "")
    {
        this->stopped = true;
        this->log(msg);
    }
    // Restart timer (does not unpause)
    void restart() { this->timer = clock_t::now(); }
    // Log and restart timer with optional additional label
    void mark(std::string msg = "")
    {
        this->log(msg);
        this->restart();
    }
};

// Two-way multimap between two hashable types
template <typename TKey, typename TValue> class Bimap
{
   private:
    std::unordered_map<TKey, std::unordered_set<TValue>> map;
    std::unordered_map<TValue, std::unordered_set<TKey>> revMap;

   public:
    Bimap() = default;

    std::unordered_set<TValue>& at(const TKey& key)
    {
        auto it = this->map.find(key);
        if (it == this->map.end()) {
            it = this->map.try_emplace(key).first;
        }
        return it->second;
    }
    std::unordered_set<TValue> at(const TKey& key) const
    {
        auto it = this->map.find(key);
        if (it == this->map.end()) {
            return {};
        } else {
            return it->second;
        }
    }
    std::unordered_set<TKey>& at(const TValue& value)
    {
        auto it = this->revMap.find(value);
        if (it == this->revMap.end()) {
            it = this->revMap.try_emplace(value).first;
        }
        return it->second;
    }
    std::unordered_set<TKey> at(const TValue& value) const
    {
        auto it = this->revMap.find(value);
        if (it == this->revMap.end()) {
            return {};
        } else {
            return it->second;
        }
    }
    bool contains(const TKey& key) const
    {
        return this->map.contains(key) && !this->map.at(key).empty();
    }
    bool contains(const TValue& value) const
    {
        return this->revMap.contains(value) && !this->revMap.at(value).empty();
    }
    bool contains(const TKey& key, const TValue& value) const
    {
        return this->map.contains(key) && this->revMap.contains(value);
    }
    void erase(const TKey& key)
    {
        for (const TValue& value : this->map.at(key)) {
            this->revMap.at(value).erase(key);
        }
        this->map.erase(key);
    }
    void erase(const TValue& value)
    {
        for (const TKey& key : this->revMap.at(value)) {
            this->map.at(key).erase(value);
        }
        this->revMap.erase(value);
    }
    void erase(const TKey& key, const TValue& value)
    {
        this->map.at(key).erase(value);
        this->revMap.at(value).erase(key);
    }
    void clear()
    {
        this->map.clear();
        this->revMap.clear();
    }
    void insert(const TKey& key, const TValue& value)
    {
        this->map.try_emplace(key).first->second.insert(value);
        this->revMap.try_emplace(value).first->second.insert(key);
    }
    size_t size() const { return this->map.size(); }
    bool empty() const { return this->map.empty(); }
};

// Different concepts that provide different interfaces for automatically tracking modifications
template <typename T>
concept c_LazyType = std::ranges::random_access_range<T> || std::is_class_v<T>;

// Lazy wrapper base class that holds common methods
template <c_LazyType T> class LazyBase
{
   protected:
    T obj;
    // Flag that indicates the object has been modified since last clear()
    bool isModified = true;

   public:
    LazyBase() = default;
    LazyBase(T obj) : obj(obj) {}

    T& operator*() { return this->obj; }
    const T& operator*() const { return this->obj; }
    T* operator->() { return &this->obj; }
    const T* operator->() const { return &this->obj; }

    // Returns true if the obj has been modified since last clear()
    bool modified() const { return this->isModified; }
    // Sets modified flag to given obj
    void modified(bool flag) { this->isModified = flag; }
    // Sets modified flag to false
    void invalidate() { this->isModified = true; }
    // Sets modified flag to false and returns true if it was not already false
    bool check()
    {
        if (this->isModified) {
            this->isModified = false;
            return true;
        }
        return false;
    }
};

// Wrap type T with a flag that indicates modifications that need handling by some system
template <typename T> class Lazy : public LazyBase<T>
{
   public:
    using LazyBase<T>::LazyBase;
    using LazyBase<T>::obj;
    using LazyBase<T>::isModified;
};

template <std::ranges::range R> class Lazy<R> : public LazyBase<R>
{
   public:
    using LazyBase<R>::LazyBase;
    using LazyBase<R>::obj;
    using LazyBase<R>::isModified;

    // Access operator that does not change the modified flag
    const auto& operator[](const auto& k) const { return this->obj.at(k); }
    // Access operator that is not const and does change the modified flag
    auto& operator[](const auto& k) { return this->obj[k]; }
    // Returns result of
    size_t size() const { return std::ranges::size(this->obj); }
    // Calls clear on the object
    void clear() { this->obj.clear(); }
};

// Constexpr random number generator
class random
{
   private:
    static std::mt19937 generator;

    static constexpr uint64_t constseed()
    {
        uint64_t shifted = 0;
        for (const char c : __TIME__) {
            shifted = (shifted << 8) | c;
        }
        return shifted;
    }

    template <std::unsigned_integral TValue> struct PCG
    {
        struct pcg32_random_t
        {
            uint64_t state = 0;
            uint64_t inc = constseed();
        } rng;

        constexpr TValue operator()() { return pcg32_random_r(); }
        static TValue constexpr min() { return std::numeric_limits<TValue>::min(); }
        static TValue constexpr max() { return std::numeric_limits<TValue>::max(); }

       private:
        constexpr uint32_t pcg32_random_r()
        {
            uint64_t oldstate = this->rng.state;
            // Advance internal state
            this->rng.state = oldstate * 6364136223846793005ULL + (this->rng.inc | 1);
            // Calculate output function (XSH RR), uses old state for max ILP
            uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
            uint32_t rot = oldstate >> 59u;
            return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
        }
    };

   public:
    template <std::unsigned_integral TValue> static constexpr TValue crand(int count = 10)
    {
        PCG<TValue> pcg;
        while (count + 1 > 0) {
            pcg();
            --count;
        }

        return pcg();
    }

    template <std::unsigned_integral TValue> static constexpr TValue crand(const char* name)
    {
        int n = 0;
        for (const char* c = name; *c; c++) {
            n += *c;
        }
        PCG<TValue> pcg;
        pcg.rng.inc = n;
        for (int i = 0; i < 10; i++) {
            pcg();
        }
        return pcg();
    }

    // Generate random integer in range [min, max]
    template <std::integral T> static T gen(T min, T max)
    {
        return std::uniform_int_distribution<T>(min, max)(generator);
    }
    // Generate random integer in range [0, max]
    template <std::integral T> static T gen(T max) { return random::gen<T>(T(0), max); }
    // Generate random integer of any possible value > 0 for type T
    template <std::integral T> static T gen()
    {
        return random::gen<T>(std::numeric_limits<T>::max());
    }
    // Generate random real num in range [min, max]
    template <std::floating_point T> static T gen(T min, T max)
    {
        return std::uniform_real_distribution<T>(min, max)(generator);
    }
};

// Smart index type base class, uniquely identifies its type
template <uint32_t _tag> class IndexType
{
   private:
    inline void tag(uint32_t tag) { this->bytes = (((uint64_t)tag) << 32) | this->idx(); }

   public:
    // The index data
    uint64_t bytes = 0u;
    constexpr static uint32_t tagval = _tag;

    static IndexType<_tag> random() { return IndexType<_tag>(random::gen<uint32_t>()); }

    IndexType(uint32_t idx = 0u)
    {
        this->tag(_tag);
        this->idx(idx);
    }
    IndexType(uint64_t bytes) : bytes(bytes)
    {
        if (this->tag() == 0u) {
            this->tag(_tag);
        }
        $assert(this->valid(), "tag {} != {}", this->tag(), _tag);
    }

    static inline bool valid(uint64_t bytes) { return (bytes >> 32) == _tag; }

    inline void idx(uint32_t idx) { this->bytes = (((uint64_t)this->tag()) << 32) | idx; }
    inline uint32_t idx() const { return this->bytes & 0xFFFFFFFFul; }
    inline uint32_t tag() const { return this->bytes >> 32; }
    inline bool valid() const { return this->tag() == _tag; }
    inline bool operator==(const IndexType<_tag>& i) const { return this->idx() == i.idx(); }
    inline bool operator<(const IndexType<_tag>& i) const { return this->idx() < i.idx(); }
    inline bool operator>(const IndexType<_tag>& i) const { return this->idx() > i.idx(); }

    /* hashable */ size_t _hash() const { return this->bytes; }
    /* printable */ std::string _format() const
    {
        return fmt::format("{}({})", this->bytes, this->idx());
    }
};

#define $index_type(name) IndexType<random::crand<uint32_t>((name))>

// Iterates over an aligned range of bytes where the size of type might be different
template <typename T> class StrideIterator
{
   public:
    /* IteratorTraits */ using difference_type = std::ptrdiff_t;
    /* IteratorTraits */ using value_type = T;
    /* IteratorTraits */ using pointer = T*;
    /* IteratorTraits */ using reference = T&;
    /* IteratorTraits */ using iterator_category = std::random_access_iterator_tag;
    /* IteratorTraits */ using iterator_concept = std::random_access_iterator_tag;

   protected:
    std::byte* ptr;
    std::ptrdiff_t stride;

    using itr_t = StrideIterator<T>;

    inline T* get(std::ptrdiff_t n = 0) const
    {
        return reinterpret_cast<T*>(this->ptr + n * this->stride);
    }
    inline itr_t& set(std::ptrdiff_t n)
    {
        this->ptr += n * this->stride;
        return *this;
    }

   public:
    StrideIterator(const T* ptr, std::ptrdiff_t stride = sizeof(T))
        : ptr((std::byte*)(ptr)), stride(stride)
    {
        assert(stride >= sizeof(T));
    }
    StrideIterator() : StrideIterator(nullptr){};

    inline itr_t& operator+=(std::ptrdiff_t n) { return this->set(n); }
    inline itr_t& operator-=(std::ptrdiff_t n) { return this->set(-n); }
    inline itr_t& operator++() { return this->set(1); }
    inline itr_t& operator--() { return this->set(-1); }
    inline itr_t operator++(int) const { return { this->get(1), stride }; };
    inline itr_t operator--(int) const { return { this->get(-1), stride }; };

    inline T& operator[](std::ptrdiff_t n) const { return *this->get(n); }

    inline T& operator*() const { return *this->get(); }
    inline T* operator->() const { return this->get(); }

    inline bool operator==(const itr_t& i) const { return this->ptr == i.ptr; }
    inline bool operator!=(const itr_t& i) const { return this->ptr != i.ptr; }
    inline bool operator<(const itr_t& i) const { return this->ptr < i.ptr; }
    inline bool operator>(const itr_t& i) const { return this->ptr > i.ptr; }
    inline bool operator<=(const itr_t& i) const { return this->ptr <= i.ptr; }
    inline bool operator>=(const itr_t& i) const { return this->ptr >= i.ptr; }

    inline itr_t operator+(std::ptrdiff_t n) const { return { this->get(n), stride }; }
    inline itr_t operator-(std::ptrdiff_t n) const { return { this->get(-n), stride }; }

    friend inline itr_t operator+(std::ptrdiff_t n, const itr_t& itr)
    {
        return { itr.get(n), itr.stride };
    }
    friend inline itr_t operator-(std::ptrdiff_t n, const itr_t& itr)
    {
        return { itr.get(-n), itr.stride };
    }

    inline std::ptrdiff_t operator-(const itr_t& i) const
    {
        return (this->ptr - i.ptr) / (std::ptrdiff_t)this->stride;
    }
};

static_assert(std::weakly_incrementable<StrideIterator<int>>);
