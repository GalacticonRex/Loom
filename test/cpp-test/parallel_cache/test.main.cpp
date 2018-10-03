#include <thread>
#include <chrono>
#include <cstdio>

void __thread_local(int value) {
    for ( int i=0;i<value;i++ )
    for ( int j=0;j<value;j++ );
}
void __thread_extern(int value, int& i, int& j) {
    for ( i=0;i<value;i++ )
    for ( j=0;j<value;j++ );
}

int main(int argc, char** argv) {
    int value = 100000;

    {
        auto start = std::chrono::high_resolution_clock::now();

        std::thread pool[4];

        for ( int i=0;i<4;i++ ) {
            pool[i] = std::thread(__thread_local, value);
        }

        for ( int i=0;i<4;i++ ) {
            pool[i].join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> dur = end - start;
        
        std::printf("Execution took %f seconds\n\n", dur.count());
    }

    {
        auto start = std::chrono::high_resolution_clock::now();

        int buffer[6];
        std::thread pool[3];

        for ( int i=0;i<3;i++ ) {
            pool[i] = std::thread( __thread_extern,
                                   value,
                                   std::ref(buffer[i*2+0]),
                                   std::ref(buffer[i*2+1]) );
        }

        regenerate:
        for ( int i=0;i<6;i++ )
            if ( buffer[i] < value )
                goto regenerate;

        for ( int i=0;i<3;i++ ) {
            pool[i].join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> dur = end - start;

        std::printf("Execution took %f seconds\n\n", dur.count());
    }


    return 0;
}