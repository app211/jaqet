#ifndef QSIGNALBLOCKER
#define QSIGNALBLOCKER

template<class T> class Blocker {
    T *blocked;
    bool previous;
public:
    Blocker(T *blocked)
        : blocked(blocked),
          previous(blocked->blockSignals(true)) {}
    ~Blocker() { blocked->blockSignals(previous); }
    T *operator->() { return blocked; }
};

template<class T> inline Blocker<T> whileBlocking(T *blocked) {
    return Blocker<T>(blocked);
}

template<class T,typename Functor>
void whileBlocking(T *blocked,Functor f)
{
    bool previous=blocked->blockSignals(true);
    f();
    blocked->blockSignals(previous);
}
#endif // QSIGNALBLOCKER
