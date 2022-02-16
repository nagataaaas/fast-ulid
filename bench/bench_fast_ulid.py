import datetime
import time

import fast_ulid

test_count = 1000000

if __name__ == '__main__':

    ul = str(fast_ulid.ulid())
    dt = datetime.datetime.now()
    ts = datetime.datetime.now().timestamp()

    tests = [('create str: ', 'str(fast_ulid.ulid())'),
             ('create str with datetime: ', 'str(fast_ulid.ulid(dt))'),
             ('create str with timestamp: ', 'str(fast_ulid.ulid(ts))'),
             ('decode datetime from str: ', 'fast_ulid.decode_datetime(ul)'),
             ('decode timestamp from str: ', 'fast_ulid.decode_timestamp(ul)')]

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
