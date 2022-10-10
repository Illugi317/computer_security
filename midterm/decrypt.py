ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
string_to_decode = "OLZLJYLATLZZHNLPZHNYLLHISLULZZLZ"
#DECODED_MESSAGE = "HESECRETMESSAGEISAGREEABLENESSES"

english_letter_frequencies = {'E': 12.70, 'T': 9.06, 'A': 8.17, 'O': 7.51, 'I': 6.97, 'N': 6.75, 'S': 6.33, 'H': 6.09, 'R': 5.99, 'D': 4.25, 'L': 4.03, 'C': 2.78, 'U': 2.76, 'M': 2.41, 'W': 2.36, 'F': 2.23, 'G': 2.02, 'Y': 1.97, 'P': 1.93, 'B': 1.29, 'V': 0.98, 'K': 0.77, 'J': 0.15, 'X': 0.15, 'Q': 0.10, 'Z': 0.07}
bigram_frequencies = {
"TH": 1.52,       "EN": 0.55,       "NG": 0.18,
"HE": 1.28,       "ED": 0.53,       "OF": 0.16,
"IN": 0.94,       "TO": 0.52,       "AL": 0.09,
"ER": 0.94,       "IT": 0.50,       "DE": 0.09,
"AN": 0.82,       "OU": 0.50,       "SE": 0.08,
"RE": 0.68,       "EA": 0.47,       "LE": 0.08,
"ND": 0.63,       "HI": 0.46,       "SA": 0.06,
"AT": 0.59,       "IS": 0.46,       "SI": 0.05,
"ON": 0.57,       "OR": 0.43,       "AR": 0.04,
"NT": 0.56,       "TI": 0.34,       "VE": 0.04,
"HA": 0.56,       "AS": 0.33,       "RA": 0.04,
"ES": 0.56,       "TE": 0.27,       "LD": 0.02,
"ST": 0.55,       "ET": 0.19,       "UR": 0.02,
}

trigram_frequencies = {
"THE":  1.81,        "ERE" :  0.31,        "HES" :  0.24,
"AND":  0.73,        "TIO" :  0.31,        "VER" :  0.24,
"ING":  0.72,        "TER" :  0.30,        "HIS" :  0.24,
"ENT":  0.42,        "EST" :  0.28,        "OFT" :  0.22,
"ION":  0.42,        "ERS" :  0.28,        "ITH" :  0.21,
"HER":  0.36,        "ATI" :  0.26,        "FTH" :  0.21,
"FOR":  0.34,        "HAT" :  0.26,        "STH" :  0.21,
"THA":  0.33,        "ATE" :  0.25,        "OTH" :  0.21,
"NTH":  0.33,        "ALL" :  0.25,        "RES" :  0.21,
"INT":  0.32,        "ETH" :  0.24,        "ONT" :  0.20,
}
def frequency_analysis(string):
    freq = {x: 0 for x in ALPHABET}
    for letter in string:
        if letter in freq:
            freq[letter] += 1
        else:
            freq[letter] = 1
    return freq

def score_shift(cipher_text, shift): #gives a weighted score
    s = 0
    freq_dict = frequency_analysis(cipher_text)
    for x in ALPHABET:
        s_index = (ALPHABET.find(x) + shift) % 26
        s += abs(freq_dict[x] - english_letter_frequencies[ALPHABET[s_index]])
    return s/26

def all_shifts(string):
    shift_list = []
    for x in range(len(ALPHABET)):
        shifted = ''
        for letter in string:
            shifted += ALPHABET[(ALPHABET.index(letter) + x) % 26]
        shift_list.append(shifted)
    return shift_list

def bigram_score(string):
    score = 0
    for i in range(len(string)-1):
        if string[i:i+2] in bigram_frequencies:
            score += bigram_frequencies[string[i:i+2]]
    return score

def trigram_score(string):
    score = 0
    for i in range(len(string)-2):
        if string[i:i+3] in trigram_frequencies:
            score += trigram_frequencies[string[i:i+3]]
    return score

if '__main__' == __name__:
    #Try to decode the fucker
    shift_list = all_shifts(string_to_decode)
    print("-----------------------")
    over_90 = []
    for idx, x in enumerate(shift_list):
        if score_shift(x, idx) > 2:
            over_90.append((x,score_shift(x, idx)))
    #check bigram score
    over_5 = []
    for x in over_90:
        if bigram_score(x[0]) > 3.5:
            over_5.append(x)
    #check trigram score
    trigram =[]
    for x in over_5:
        trigram.append((x[0], trigram_score(x[0])))
    #get the highest trigram score with string
    highest_string = ''
    highest = 0
    for x in trigram:
        if x[1] > highest:
            highest = x[1]
            highest_string = x[0]
    
    print(f"Key: {shift_list.index(highest_string)}")
    print(f"Decoded: {highest_string}")