#! /usr/bin/python

import shutil


def main():
    for i in range(10):
        new_file_name = "DummyOpponent{}.{}"
        old_name = "DummyOpponent.{}"
        extensions = ["h", "cpp"]
        for extension in extensions:
            new_id = str(i) * 9
            new_file = new_file_name.format(new_id, extension)
            shutil.copy(old_name.format(extension), new_file)
            
            with open(new_file, "a+t") as f:
                data = f.read()
                data = data.replace("123456789", new_id)
                data = data.replace("DummyOpponent.h", "DummyOpponent{}.h".format(new_id))
                f.seek(0)
                f.truncate()
                f.write(data)

if __name__ == "__main__":
    main()

