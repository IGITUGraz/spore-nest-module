cd .docs
doxygen
git add *
git commit -m "Update docs to revision `git rev-parse HEAD`."
echo "Entering sub-shell. You can now check the docs for integrity. Press CTRL+D to continue."
$SHELL
read -p "Push commit? [Y/n] " answer
case $answer in
    [Yy]* ) ;;
    [Nn]* ) exit;;
    * ) ;;
esac
git push --set-upstream origin gh-pages

