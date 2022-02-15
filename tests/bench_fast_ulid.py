import datetime
import time

import fast_ulid

test_count = 1000000

if __name__ == '__main__':

    ul = str(fast_ulid.ulid())
    dt = datetime.datetime.now()
    ts = datetime.datetime.now().timestamp()

    tests = [('create str: ', lambda: str(fast_ulid.ulid())),
             ('create str with datetime: ', lambda: str(fast_ulid.ulid(dt))),
             ('create str with timestamp: ', lambda: str(fast_ulid.ulid(ts))),
             ('decode datetime from str: ', lambda: fast_ulid.decode_datetime(ul)),
             ('decode timestamp from str: ', lambda: fast_ulid.decode_time(ul))]

    for title, func in tests:
        res = []
        for _ in range(10):
            start = time.time()
            for _ in range(test_count):
                _ = func()
            res.append(time.time() - start)
        print(title, test_count, ': ', res)
