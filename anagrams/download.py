import string
import urllib.request
import os
import os.path
import shutil
import gzip

def download_data():
    base = 'http://storage.googleapis.com/books/ngrams/books/googlebooks-eng-all-1gram-20120701-{letter}.gz'
    os.makedirs('data', exist_ok=True)

    for l in string.ascii_lowercase:
        out_path = f'data/{l}'

        if os.path.isfile(out_path):
            continue

        response = urllib.request.urlopen(base.format(letter=l))
        with open(out_path, 'wb') as f:
            f.write(gzip.decompress(response.read()))

        print(f'Downloaded {out_path}')

download_data()
print('Downloaded data')