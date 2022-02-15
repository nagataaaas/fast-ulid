import datetime
import time

import ulid

test_count = 1000000

if __name__ == '__main__':

    ul = str(ulid.new())
    dt = datetime.datetime.now()
    ts = datetime.datetime.now()

    tests = [('create str: ', lambda: str(ulid.new())),
             ('create str with datetime: ', lambda: str(ulid.from_timestamp(dt))),
             ('create str with timestamp: ', lambda: str(ulid.from_timestamp(ts))),
             ('decode datetime from str: ', lambda: ulid.from_str(ul).timestamp().datetime),
             ('decode timestamp from str: ', lambda: ulid.from_str(ul).timestamp().int)]

    for title, func in tests:
        res = []
        for _ in range(10):
            start = time.time()
            for _ in range(test_count):
                _ = func()
            res.append(time.time() - start)
        print(title, test_count, ': ', res)
