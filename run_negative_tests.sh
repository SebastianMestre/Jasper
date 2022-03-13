
for t in tests/should_fail/*.jp
do
	./bin/jasperi $t 2> /dev/null > /dev/null
	if [ $? -eq 0 ]
	then
		echo $(printf "\\u001b[31mFAIL: $t exited with a 'success' code\\u001b[0m");
	fi
done
