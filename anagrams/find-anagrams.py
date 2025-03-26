import csv

data = []
reader = csv.reader(open('data/hom-sorted.csv', encoding='utf8'))
for row in reader:
    data.append(sorted(row[0]), row[0])

def find_anagrams(word):
    for (sorted_word, match_word) in data:
        if sorted(word) == sorted_word and word != match_word:
            print(match_word)
