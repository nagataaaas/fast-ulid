#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <datetime.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <random>
#include <string.h>
#include <string>

#define ENCODING "0123456789ABCDEFGHJKMNPQRSTVWXYZ"
#define ENCODING_LEN 32
#define ENCODING_BIT_LEN 5
#define TIME_LEN 10
#define RANDOM_LEN 16

#if PY_VERSION_HEX < 0x03070000
PyObject* timezone_utc;
#define UTC_SINGLETON timezone_utc
void create_utc_tz() {
  PyObject* pytz = PyImport_ImportModule("datetime");
  if (!pytz) {
    return;
  }
  PyObject* pytz_utc = PyObject_GetAttrString(pytz, "timezone");
  Py_DECREF(pytz);
  if (!pytz_utc) {
    return;
  }
  PyObject* pytz_utc_utc = PyObject_GetAttrString(pytz_utc, "utc");
  Py_DECREF(pytz_utc);
  timezone_utc = pytz_utc_utc;
  Py_DECREF(pytz_utc_utc);
}
#define CALLER_HEAD(f) (PyCFunction)f
#define METH_MAYBE_FASTCALL METH_VARARGS
#else
#define UTC_SINGLETON PyDateTime_TimeZone_UTC
#define CALLER_HEAD(f) (PyCFunction)(void(*)(void))f
#define METH_MAYBE_FASTCALL METH_FASTCALL
#endif

std::random_device rnd;
unsigned long long lastTime = 0;
uint8_t *lastRandom = nullptr;
const int perSingleRandom = (sizeof(unsigned int) * 8 / 5);
const int codepointToValue[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13,
                                14, 15, 16, 17, -1, 18, 19, -1, 20, 21, -1, 22, 23, 24, 25, 26, -1, 27, 28,
                                29, 30, 31, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, 16, 17, -1, 18, 19,
                                -1, 20, 21, -1, 22, 23, 24, 25, 26, -1, 27, 28, 29, 30, 31};


bool incrementRandomBytes(uint8_t *random) {
    random[RANDOM_LEN - 1]++;
    for (int i = RANDOM_LEN - 1; i >= 0; i--) {
        if (random[i] == 32 && i != 0) {
            random[i] = 0;
            random[i - 1]++;
        } else if (random[i] == 32 && i == 0) {
            PyErr_SetString(PyExc_OverflowError, "Random number overflow");
            return false;
        } else {
            break;
        }
    }
    return true;
}


uint8_t *random_bytes(void) {
    uint8_t *bytes = new uint8_t[RANDOM_LEN];
    unsigned int random;
    for (int i = 0; i < std::ceil(80.0 / perSingleRandom / 5); i++) {
        random = rnd();
        for (int j = 0; j < perSingleRandom; j++) {
            bytes[i * perSingleRandom + j] = random & 0x1f;
            random >>= 5;
            if (i * perSingleRandom + j == RANDOM_LEN - 1) {
                break;
            }
        }
    }
    return bytes;
}

unsigned long long getTimestamp() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

uint8_t *getRandom(unsigned long long timestamp) {
    if (lastTime >= timestamp) {
        if (!incrementRandomBytes(lastRandom)){
            // failed increment
            return nullptr;
        }
        return lastRandom;
    }
    lastTime = timestamp;
    if (lastRandom != nullptr) {
        delete[] lastRandom;
    }
    uint8_t *random = random_bytes();
    lastRandom = random;
    return random;
}


void encodeTime(unsigned long long time, char timestamp[]) {
    int mod;
    for (int i = TIME_LEN - 1; i >= 0; i--) {
        mod = time % ENCODING_LEN;
        timestamp[i] = ENCODING[mod];
        time = time >> ENCODING_BIT_LEN;
    }
}

void encodeRandom(uint8_t *random, char randomness[]) {
    for (int i = 0; i < RANDOM_LEN; i++) {
        randomness[i] = ENCODING[random[i]];
    }
}

char *ULID(unsigned long long time = 0) {
    uint8_t *randoms;
    char Timestamp[TIME_LEN + 1];
    char Randomness[RANDOM_LEN + 1];
    if (time == 0) {
        time = getTimestamp();
    }
    randoms = getRandom(time);
    if (randoms == nullptr) {
        // failed to get random (failed to increment)
        return nullptr;
    }

    encodeTime(time, Timestamp);
    encodeRandom(randoms, Randomness);

    char *ulid = new char[TIME_LEN + RANDOM_LEN + 1];
    for (int i = 0; i < TIME_LEN; i++) {
        ulid[i] = Timestamp[i];
    }
    for (int i = 0; i < RANDOM_LEN; i++) {
        ulid[TIME_LEN + i] = Randomness[i];
    }
    ulid[TIME_LEN + RANDOM_LEN] = '\0';
    return ulid;
}


inline int char_index(char c) {
    if (c < '0' || c > 'z') {
        return -1;
    }
    return codepointToValue[c - '0'];
}


double decodeTime(const char* ulid) {
    if (strlen(ulid) != TIME_LEN + RANDOM_LEN) {
        char error[40];
        sprintf(error, "Invalid length of ULID: %zd", strlen(ulid));

        PyErr_SetString(PyExc_ValueError, std::string(error).c_str());
        return -1;
    }
    if (char_index(ulid[0]) >= 8) {
        char error[100];
        sprintf(error, "Invalid time in ULID: `%s`. timestamp must be less than 2 ^ 48", ulid);

        PyErr_SetString(PyExc_OverflowError, std::string(error).c_str());
        return -1;
    }
    unsigned long long time = 0;
    for (int i = 0; i < TIME_LEN; i++) {
        int index = char_index(ulid[i]);
        if (index == -1) {
            char error[65];
            sprintf(error, "Invalid character `%c` in ULID: `%s`", ulid[i], ulid);

            PyErr_SetString(PyExc_ValueError, std::string(error).c_str());
            return -1;
        }
        time = (time << ENCODING_BIT_LEN) + index;
    }
    return (double)time * 0.001;
}


static char ulid_docs[] = "ulid(timestamp: datetime.datetime | float = None)-> str: \nReturn ULID. if timestamp is given, create ULID with it.\n";
static char decode_datetime_docs[] = "decode_datetime(ulid: str)-> datetime.datetime: \nReturn timestamp of ULID in `datetime.datetime`.\nTimestamp can cause overflow, so if it's too large, please use `decode_timestamp` instead.\n";
static char decode_timestamp_docs[] = "decode_timestamp(ulid: str)-> float: \nReturn timestamp of ULID in `float`.\n";


#if PY_VERSION_HEX < 0x03070000
#include "ULID_below_36.cpp"
#else
#include "ULID_above_37.cpp"
#endif


static PyMethodDef ulid_module_methods[] = {
        {"ulid",        CALLER_HEAD(ulid_py),        METH_MAYBE_FASTCALL | METH_KEYWORDS, ulid_docs},
        {"decode_datetime", CALLER_HEAD(decode_datetime_py), METH_MAYBE_FASTCALL | METH_KEYWORDS, decode_datetime_docs},
        {"decode_timestamp", CALLER_HEAD(decode_timestamp_py), METH_MAYBE_FASTCALL | METH_KEYWORDS, decode_timestamp_docs},
        {NULL,          NULL,                         0,                            NULL}
};

static struct PyModuleDef ulid_module_definition = {
        PyModuleDef_HEAD_INIT,
        "ulid",
        "Extension module that provides ULID",
        -1,
        ulid_module_methods
};

static void cleanup_module(void* module) {
    if (lastRandom != nullptr) {
        delete[] lastRandom;
        lastRandom = nullptr;
    }
}

PyMODINIT_FUNC PyInit_fast_ulid(void) {
    Py_Initialize();
    PyDateTime_IMPORT;
    #if PY_VERSION_HEX < 0x03070000
    create_utc_tz();
    #endif
    PyObject* module = PyModule_Create(&ulid_module_definition);
    if (module != nullptr) {
        // Register cleanup function to be called when module is deallocated
        Py_AtExit([]() { 
            if (lastRandom != nullptr) {
                delete[] lastRandom;
                lastRandom = nullptr;
            }
        });
    }
    return module;
}