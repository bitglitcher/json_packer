import random
import string

def rand_string(N):
    return ''.join(random.choice(string.ascii_uppercase 
    + string.digits) for _ in range(N))

def gen_json(lines):
    print("Generating {} records...".format(lines))
    file1 = open("input.json", "w") 
    fmt = "\"{}\":\"{}\", \"{}\":{}, \"{}\":{}"

    for i in range(lines):
        booloean_str = ""
        if(random.randint(0, 1)):
            booloean_str = "true"
        else:
            booloean_str = "false"
            
        record = fmt.format(
            rand_string(15),
            rand_string(15),
            rand_string(15),
            random.randint(0, 0xff),
            rand_string(15),
            booloean_str)
        record = '{' + record + '}\n'
        file1.write(record)
    file1.close()

gen_json(1040)
