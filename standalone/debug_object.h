struct Announcer;

std::ostream& operator <<(std::ostream&, const Announcer&);

struct Announcer {
    int val;
    Announcer() : val(0) { clog << "Constructed blank announcer " << this << "\n"; }
    Announcer(int x) : val(x) { clog << "Constructed announcer " << this << " with " << val <<  "\n"; }
    Announcer(const Announcer& from) : val(from.val) { clog << "Copied announcer " << &from << " with " << val << " to " << this << "\n"; }
    Announcer(Announcer&& from) noexcept : val(std::exchange(from.val, 0)) { clog << "Moved announcer " << &from << " with " << val << " to " << this << "\n"; }
    ~Announcer() { clog << "Deleted announcer " << this << " with " << val << "\n"; }
};

std::ostream& operator <<(std::ostream& stream, const Announcer& announcer) {
    return stream << "Announcer(" << announcer.val << ")";
}
