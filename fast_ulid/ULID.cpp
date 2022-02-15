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
#define MAX_TIME (1ull << 48) - 1

#if _MSC_VER > 0
typedef uint32_t rand_t;
#else
typedef uint8_t rand_t;
#endif

std::random_device rnd;
unsigned long long lastTime = 0;
uint8_t *lastRandom = nullptr;
int perSingleRandom = (sizeof(unsigned int) * 8 / 5);


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

unsigned long long get_time_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
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

double decodeTime(const char* ulid){
    if (strlen(ulid) != TIME_LEN + RANDOM_LEN) {
        char error[40];
        sprintf(error, "Invalid length of ULID: %zd", strlen(ulid));

        PyErr_SetString(PyExc_ValueError, std::string(error).c_str());
        return -1;
    }
    unsigned long long time = 0;
    for (int i = 0; i < TIME_LEN; i++) {
        bool found = false;
        for (int j = 0; j < ENCODING_LEN; j++) {
            if (ENCODING[j] == ulid[i]) {
                time = (time << ENCODING_BIT_LEN) + j;
                found = true;
                break;
            }
        }
        if (!found) {
            char error[65];
            sprintf(error, "Invalid character `%c` in ULID: `%s`", ulid[i], ulid);

            PyErr_SetString(PyExc_ValueError, std::string(error).c_str());
            return -1;
        }
    }
    if (time > MAX_TIME){
        char error[100];
        sprintf(error, "Invalid time in ULID: `%s`. timestamp must be less than 2 ^ 48", ulid);

        PyErr_SetString(PyExc_OverflowError, std::string(error).c_str());
        return -1;
    }
    return (double)time * 0.001;
}

static PyObject *ulid_py(PyObject *self, PyObject *args, PyObject *kwargs) {
    PyDateTime_IMPORT;
    static char *keywords[] = {"timestamp", NULL};

    PyObject *arg = nullptr;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O", keywords, &arg)) {
        return NULL;
    }

    if (arg == nullptr) {
        auto result = ULID(NULL);
        if (result == nullptr) {
            return NULL;
        }
        return Py_BuildValue("s", result);
    }

    double timestamp;
    if (PyDateTime_Check(arg)) {
        PyObject *py_timestamp = PyObject_CallMethod(arg, "timestamp", NULL);
        timestamp = PyFloat_AsDouble(py_timestamp);
        Py_DECREF(py_timestamp);
    } else if (PyFloat_Check(arg)) {
        timestamp = PyFloat_AsDouble(arg);
    } else {
        PyErr_SetString(PyExc_TypeError, "Argument must be a instance of `datetime.datetime` or `float`");
        return NULL;
    }

    auto result = ULID(timestamp * 1000);
    if (result == nullptr) {
        return NULL;
    }
    return Py_BuildValue("s", result);
}

static PyObject *decode_datetime_py(PyObject *self, PyObject *args, PyObject *kwargs) {
    static char *keywords[] = {"ulid", NULL};

    const char *ulid;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s", keywords, &ulid)) {
        return NULL;
    }

    try {
        double timestamp = decodeTime(ulid);
        if (timestamp < 0) return NULL;
        auto datetime = PyDateTime_FromTimestamp(Py_BuildValue("(d, O)", timestamp, PyDateTime_TimeZone_UTC));
        if (datetime == NULL) {
            PyErr_SetString(PyExc_OverflowError, "Timestamp is too large to create `datetime.datetime`. Please use `decode_timestamp` instead.");
        }
        return datetime;
    } catch (std::runtime_error &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
    }
}

static PyObject *decode_timestamp_py(PyObject *self, PyObject *args, PyObject *kwargs) {
    static char *keywords[] = {"ulid", NULL};

    const char *ulid;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s", keywords, &ulid)) {
        return NULL;
    }

    try {
        double timestamp = decodeTime(ulid);
        if (timestamp < 0) return NULL;
        return Py_BuildValue("d", timestamp);
    } catch (std::runtime_error &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
    }
}

static char ulid_docs[] = "ulid(timestamp: datetime.datetime | float = None)-> str: \nReturn ULID. if timestamp is given, create ULID with it.\n";
static char decode_datetime_docs[] = "decode_datetime(ulid: str)-> datetime.datetime: \nReturn timestamp of ULID in `datetime.datetime`.\nTimestamp can cause overflow, so if it's too large, please use `decode_timestamp` instead.\n";
static char decode_timestamp_docs[] = "decode_timestamp(ulid: str)-> float: \nReturn timestamp of ULID in `float`.\n";

static PyMethodDef ulid_module_methods[] = {
        {"ulid",        (PyCFunction) ulid_py,        METH_VARARGS | METH_KEYWORDS, ulid_docs},
        {"decode_datetime", (PyCFunction) decode_datetime_py, METH_VARARGS | METH_KEYWORDS, decode_datetime_docs},
        {"decode_timestamp", (PyCFunction) decode_timestamp_py, METH_VARARGS | METH_KEYWORDS, decode_timestamp_docs},
        {NULL,          NULL,                         0,                            NULL}
};

static struct PyModuleDef ulid_module_definition = {
        PyModuleDef_HEAD_INIT,
        "ulid",
        "Extension module that provides ULID",
        -1,
        ulid_module_methods
};

PyMODINIT_FUNC PyInit_fast_ulid(void) {
    Py_Initialize();
    return PyModule_Create(&ulid_module_definition);
}