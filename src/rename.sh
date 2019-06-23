for f in *.c; do 
	mv -- "$f" "${f%.c}.cpp"
done

for f in *.h; do 
	mv -- "$f" "${f%.h}.hpp"
done
