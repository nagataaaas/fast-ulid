import datetime
import time

import ulid

test_count = 1000000

if __name__ == '__main__':

    ul = str(ulid.ULID())
    dt = datetime.datetime.now()
    ts = datetime.datetime.now().timestamp()

    tests = [('create str: ', 'str(ulid.ULID())'),
             ('create str with datetime: ', 'str(ulid.ULID.from_datetime(dt))'),
             ('create str with timestamp: ', 'str(ulid.ULID.from_timestamp(ts))'),
             ('decode datetime from str: ', 'ulid.ULID.from_str(ul).datetime'),
             ('decode timestamp from str: ', 'ulid.ULID.from_str(ul).timestamp')]

    for title, func in tests:
        code = f'''
res = []
for _ in range(10):
    start = time.time()
    for _ in range(test_count):
        _ = {func}
    res.append(time.time() - start)
print('{title}', test_count, ': ', res)
    '''
        eval(compile(code, '<string>', 'exec'))
