#include <gcrypt.h>
#include <stdio.h>
#include <time.h>
#include <thread>
#include <unistd.h>

using namespace std;

#define SIZE 19

char passwords[SIZE][41] = {
    "6fa2000398eaf94788c238f3ef366ee6a846b7cd", //4b
    "3619c5f12e82c95f41ca1dda22598d076cba4f1e",
    "ccc4f07c7f2aea5e67d7ae31403a4778d4fad9d3",
    "0c2ada2d03d5859266815023ba0d0a7063816bc5", //nofear
    "3d7b4f23b8f853910e4c64f09cdf897a59db524a", //1989
    "8beace1a81284fd1228c23889b7f90ecde9b70e6", //zami
    "dbd122ef7b6a09ffecf5db9c9296320f3c94e707", //andi
    "2f571b8c86f5104a083f68bddb85983a14de3177",
    "efa44987b6e36a90882a7df93eedc89343acdcb6", //van
    "9adbe81da58f0c9eb35d72de84c4613d2b0e3b5a", //ocp
    "2d11dd4f8d181136bbf06f5d3730d5472110a08d", //nocsak
    "d5bf7da93cfc6d3fa1bfe6b15813e083578083a6", //siamese
    "e1a8bfef35fcd6fa31b83f26ab7ab1d4cddc3df7",
    "806537edf27deb913dc8e05cafb7a6e1165780ee", //kormi
    "074938e501b01f896afac8f9aa38d4959410466c", //macika
    "90e3cb213ae885288fdcd535685636b0692c24ef", //turcsi
    "cfdd72da3510d74c5d098879ee721aede700e8b7", //janika
    "02ce6cc501779139fce5dfa328bbf164518d0b80",
    "8596d9ed9bca579819b52b3c61c26894b29b81ae"  //fka43
};

// 'a' -'z'
char alphabet1[] = {
    97,  98,  99, 100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110, 111, 112,
    113, 114, 115, 116, 117, 118, 119, 120,
    121, 122, 0
};

// '0' - '9'
char alphabet2[] = {
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 0
};

// 'A' - 'Z'
char alphabet3[] = {
    65, 66, 67, 68, 69, 70, 71, 72,
    73, 74, 75, 76, 77, 78, 79, 80,
    81, 82, 83, 84, 85, 86, 87, 88,
    89, 90, 0
};

// ' ' - '/'
char alphabet4[] = {
    32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47,
    0
};

const int hash_length = gcry_md_get_algo_dlen(GCRY_MD_SHA1);
unsigned char pwds[SIZE][20];
char z[256];
long start;

void create_pwds() {
    char a, b;
    for (int i = 0; i < SIZE; i++) {
        for(int j = 0; j < hash_length; j++) {
            a = passwords[i][j * 2];
            b = passwords[i][j * 2 + 1];
            if (a <= '9') a = a - '0'; else a = a - 'a' + 10;
            if (b <= '9') b = b - '0'; else b = b - 'a' + 10;
            pwds[i][j] = a * 16 + b;
        }
    }
}

bool check_pwd(unsigned char* pwd) {
    for (int i = 0; i < SIZE; i++) {
        if (memcmp(pwds[i], pwd, hash_length) == 0)
            return true;
    }
    return false;
}

void dumpHash(unsigned char* hash) {
    for (int i = 0; i < hash_length; ++i) {
        printf("%02x", hash[i]);
    }
    puts("");
}

void sha1(char* val, unsigned char* hash) {
    gcry_md_hash_buffer(GCRY_MD_SHA1, hash, val, strlen(val));
}

void pwd_thread(char* p, int z_length, int idx, int idx_end, int d, int digit) {
    if (d > 1) {
        for (int b = 0; b < z_length; ++b) {
            p[d - 1] = z[b];
            pwd_thread(p, z_length, idx, idx_end, d - 1, digit);
        }
    } else {
        for (int i = idx; i < idx_end && i < z_length; i++) {
            p[0] = z[i];
            //puts(p);
            unsigned char pwd[hash_length];
            sha1(p, pwd);
            if (pwd) {
                if (check_pwd(pwd)) {
                    printf("found: %s ", p);
                    dumpHash(pwd);
                }
            }
        }
    }
}

void generate_pwd_multi_thread(int n, int digit) {
    int length = strlen(z) / n;
    if ((strlen(z) % n) > 0) {
        length++;
    }
    int idx = 0;
    thread threads[n];
    char* ps[n];
    for (int t = 0; t < n; ++t) {
        char* p = (char*)calloc(digit + 1, sizeof(char));
        ps[t] = p;
        if (ps != nullptr) {
            //printf("digit: %d, length: %d, idx: %d\n", digit, length, idx);
            threads[t] = thread(pwd_thread, p, strlen(z), idx, idx + length,
                    digit, digit);
            idx += length;
            usleep(10000);
        }
    }
    for (int t = 0; t < n; ++t) {
        threads[t].join();
    }
    for (int t = 0; t < n; ++t) {
        free(ps[t]);
    }
    printf("%d char: %d sec\n", digit, (int)(time(NULL) - start));
}

int main() {
    create_pwds();

    strcpy(z, alphabet1);
    strcat(z, alphabet2);
    //strcat(z, alphabet3);
    //strcat(z, alphabet4);
    printf("alphabet length: %d\n", (int)strlen(z));

    start = time(NULL);

    unsigned int nthreads = std::thread::hardware_concurrency();
    printf("number of cores: %d\n", nthreads);
    const int N_THREADS = 3;
    const int MIN_DIGITS = 1;
    const int MAX_DIGITS = 5;
    //nthreads = N_THREADS;
    thread threads[MAX_DIGITS + 1];
    printf("number of threads: %d\n", nthreads);
    puts("");
    for (int i = MIN_DIGITS; i <= MAX_DIGITS; i++) {
        threads[i] = thread(generate_pwd_multi_thread, nthreads, i);
        usleep(100000);
    }
    for (int i = MIN_DIGITS; i <= MAX_DIGITS; i++) {
        threads[i].join();
    }

    return 0;
}
