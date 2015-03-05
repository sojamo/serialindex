cd $HOME/Documents/workspace/serialindex/Java/target/classes
jar cf ../serialindex.jar .
cp ../serialindex.jar $HOME/Documents/Processing/libraries/serialindex/library
echo "serialindex compiled on $(date)"