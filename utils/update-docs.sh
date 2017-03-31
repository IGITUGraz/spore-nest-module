# 
# This file is part of SPORE.
#
# SPORE is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# SPORE is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with SPORE.  If not, see <http://www.gnu.org/licenses/>.
#
# For more information see: https://github.com/IGITUGraz/spore-nest-module
#
# File:   update-docs.sh
# Author: Hoff
#
# Created on March 24, 2017
#

DOCS_FOLDER=.docs
COMMIT_ID=`git rev-parse HEAD`

if [ ! -d $DOCS_FOLDER ]; then
    echo "ERROR :: Build-folder for docs does not exist."
    echo "Make sure you are running this script in the right directory"
    echo " and that you have executed \`./utils/init-docs.sh\` before."
    echo "Aborting."
    exit 1
fi
cd $DOCS_FOLDER && doxygen
if [ ! $? = 0 ]; then
    echo "ERROR :: Running doxygen failed. Aborting."
    exit 2
fi
git add * && git commit -m "Update docs to revision $COMMIT_ID."
if [ ! $? = 0 ]; then
    echo "ERROR :: Creating commit failed. Aborting."
    exit 3
fi
echo "Entering sub-shell. You can now check the docs for integrity. Press CTRL+D to continue."
$SHELL
read -p "Push commit? [Y/n] " answer
case $answer in
    [Yy]* ) ;;
    [Nn]* ) exit;;
    * ) ;;
esac
git push --set-upstream origin gh-pages

