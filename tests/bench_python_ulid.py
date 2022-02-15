import time
import ulid
import datetime

test_count = 1000000

if __name__ == '__main__':
    ul = str(ulid.ULID())
    dt = datetime.datetime.now()
    ts = datetime.datetime.now()

    tests = [('create str: ', lambda: str(ulid.ULID())),
             ('create str with datetime: ', lambda: str(ulid.ULID.from_datetime(dt))),
             ('create str with timestamp: ', lambda: str(ulid.ULID.from_timestamp(ts))),
             ('decode datetime from str: ', lambda: ulid.ULID.from_str(ul).datetime),
             ('decode timestamp from str: ', lambda: ulid.ULID.from_str(ul).timestamp)]

    for title, func in tests:
        res = []
        for _ in range(10):
            start = time.time()
            for _ in range(test_count):
                _ = func()
            res.append(time.time() - start)
        print(title, test_count, ': ', res)
