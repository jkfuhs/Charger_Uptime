./Run.sh ./tests/input_1.txt >./tests/out.txt
diff -w ./tests/out.txt ./tests/input_1_expected_stdout.txt

./Run.sh ./tests/input_2.txt >./tests/out.txt
diff -w ./tests/out.txt ./tests/input_2_expected_stdout.txt

./Run.sh ./tests/input_3.txt >./tests/out.txt
diff -w ./tests/out.txt ./tests/error_expected.txt

./Run.sh ./tests/input_4.txt >./tests/out.txt
diff -w ./tests/out.txt ./tests/input_4_expected_stdout.txt

./Run.sh ./tests/input_5.txt >./tests/out.txt
diff -w ./tests/out.txt ./tests/error_expected.txt

./Run.sh ./tests/input_6.txt >./tests/out.txt
diff -w ./tests/out.txt ./tests/error_expected.txt

./Run.sh ./tests/input_7.txt >./tests/out.txt
diff -w ./tests/out.txt ./tests/error_expected.txt

./Run.sh ./tests/input_8.txt >./tests/out.txt
diff -w ./tests/out.txt ./tests/input_8_expected_stdout.txt

./Run.sh ./tests/input_9.txt >./tests/out.txt
diff -w ./tests/out.txt ./tests/input_9_expected_stdout.txt

./Run.sh ./tests/input_10.txt >./tests/out.txt
diff -w ./tests/out.txt ./tests/error_expected.txt

rm -f ./tests/out.txt