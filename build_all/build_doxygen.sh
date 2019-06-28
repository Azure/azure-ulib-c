#!/bin/bash

script_dir=$(cd "$(dirname "$0")" && pwd)
repo_dir=$script_dir/..

cd $repo_dir

if [ ! -d doxy/ ]; then
    mkdir doxy
fi

pushd doxy
for filename in $repo_dir/inc/*.h
do
    if [ ! -d inc/ ]; then
        mkdir inc
    fi
    file=${filename##*/}
    perl -0pe 's/MU_DEFINE_ENUM\(\s*?(\w*),([\s\w\W]*)\)/typedef enum \1_TAG {\2} \1;/g;
                s/MOCKABLE_FUNCTION\(, (.*?), (.*?)(\);)/\1 \2();/g;
                s/MOCKABLE_FUNCTION\(, (.*?), (.*?),\s*([\w\*_\s]*), (\w*)\);/\1 \2(\3 \4);/g;
                s/MOCKABLE_FUNCTION\(, (.*?), (.*?),\s*([\w\*_\s]*), (\w*),\s*([\w\*_\s]*), (\w*)\);/\1 \2(\3 \4, \5 \6);/g;
                s/MOCKABLE_FUNCTION\(, (.*?), (.*?),\s*([\w\*_\s]*), (\w*),\s*([\w\*_\s]*), (\w*),\s*([\w\*_\s]*), (\w*)\);/\1 \2(\3 \4, \5 \6, \7 \8);/g;
                s/MOCKABLE_FUNCTION\(, (.*?), (.*?),\s*([\w\*_\s]*), (\w*),\s*([\w\*_\s]*), (\w*),\s*([\w\*_\s]*), (\w*),\s*([\w\*_\s]*), (\w*)\);/\1 \2(\3 \4, \5 \6, \7 \8, \9 \10);/g' $filename > inc/$file
done

for filename in $repo_dir/config/*.h
do
    if [ ! -d config/ ]; then
        mkdir config
    fi
    file=${filename##*/}
    perl -0pe 's/MU_DEFINE_ENUM\(\s*?(\w*),([\s\w\W]*)\)/typedef enum \1_TAG {\2} \1;/g;
                s/MOCKABLE_FUNCTION\(, (.*?), (.*?)(\);)/\1 \2();/g;
                s/MOCKABLE_FUNCTION\(, (.*?), (.*?),\s*([\w\*_\s]*), (\w*)\);/\1 \2(\3 \4);/g;
                s/MOCKABLE_FUNCTION\(, (.*?), (.*?),\s*([\w\*_\s]*), (\w*),\s*([\w\*_\s]*), (\w*)\);/\1 \2(\3 \4, \5 \6);/g;
                s/MOCKABLE_FUNCTION\(, (.*?), (.*?),\s*([\w\*_\s]*), (\w*),\s*([\w\*_\s]*), (\w*),\s*([\w\*_\s]*), (\w*)\);/\1 \2(\3 \4, \5 \6, \7 \8);/g;
                s/MOCKABLE_FUNCTION\(, (.*?), (.*?),\s*([\w\*_\s]*), (\w*),\s*([\w\*_\s]*), (\w*),\s*([\w\*_\s]*), (\w*),\s*([\w\*_\s]*), (\w*)\);/\1 \2(\3 \4, \5 \6, \7 \8, \9 \10);/g' $filename > config/$file
done

popd

pushd $repo_dir/docs
doxygen $repo_dir/docs/Doxyfile
popd

rm -r $repo_dir/doxy
