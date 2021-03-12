def read_log(name: str):
    lines=[]
    ctrs = {}
    log = []
    with open(f"C:\\Users\\JK945591\\IdeaProjects\\jeep-can-bus\\{name}") as f:
        lines = [line.rstrip() for line in f]
    # Count occurances of a code
    for line in lines:
        code_array = []
        for code_array_str in line.split(','):
            if code_array_str != '':
                code_array.append(int(code_array_str, 16))
        if len(code_array)>0:
            if not code_array[0] in ctrs:
                ctrs[code_array[0]] = 1
            else:
                ctrs[code_array[0]] += 1
            log.append(code_array)
    return ctrs, log

def substract(log_a, log_b):
    response = {}
    log_a_sorted = {k: v for k, v in sorted(log_a.items(), key=lambda item: item[1])}
    for k in log_a_sorted:
        try:
            log_b[k]
        except KeyError:
            response[k] = 1
    return response

def print_top_n_codes(n: None, log_dict):
    {k: v for k, v in sorted(log_dict.items(), key=lambda item: item[1])}
    i = 0;
    for ctr in log_dict:
        if n and i == n:
            break
        print(f"{hex(ctr)} {log_dict[ctr]}")
        i += 1
    print(f"{i} unique IDs found")

def print_messages_for(filter_code, log_array):
    for row in log_array:
        if filter_code == row[0]:
            for code in row:
                print(f"{hex(code)}", end = " ")
            print()

def print_diff_messages_for(filter_code, log_a, log_b):
    in_log_a = []
    in_log_b = []
    for row in log_a:
        if filter_code == row[0] and row not in in_log_a:
            in_log_a.append(row)
    for row in log_b:
        if filter_code == row[0] and row not in in_log_a and row not in in_log_b:
            in_log_b.append(row)
            for code in row:
                print(f"{hex(code)}", end = " ")
            print()

def summary_top_n(n):
    print("Baseline")
    ctr_base, log_base = read_log("baseline.csv")
    # print_top_n_codes(n, ctr_base)
    print()
    print("Cruise ctrl")
    ctr_cruisectrl, log_cruisectrl = read_log("cruisectrl.csv")
    # print_top_n_codes(n, ctr_cruisectrl)
    print()
    print("Stereo ctrl")
    ctr_stereo, log_stereo = read_log("stereosoundctrl.csv")
    # print_top_n_codes(n, ctr_stereo)
    print()
    print("In stereosoundctrl but not in baseline")
    ctr_substract = substract(ctr_stereo, ctr_base)
    print_top_n_codes(n, ctr_substract)
    check_code = 0x400
    print(f"All {hex(check_code)} messages baseline")
    print_messages_for(check_code,log_base)
    print(f"All {hex(check_code)} messages cruisectrl")
    print_messages_for(check_code, log_cruisectrl)
    print(f"Diff a vs b {hex(check_code)}")
    print_diff_messages_for(check_code, log_base, log_cruisectrl)

def a_summary_top_n(n):
    print("Baseline w stereo")
    ctr_base, log_base = read_log("baseline.csv")
    print_top_n_codes(n, ctr_base)
    print()
    print("Baseline w/o stereo")
    ctr_no_stereo, log_no_stereo = read_log("baseline-no-stereo.csv")
    print_top_n_codes(n, ctr_no_stereo)
    print()
    print("With vs without stereo")
    ctr_substract = substract(ctr_base, ctr_no_stereo)
    print_top_n_codes(n, ctr_substract)

a_summary_top_n(100)
