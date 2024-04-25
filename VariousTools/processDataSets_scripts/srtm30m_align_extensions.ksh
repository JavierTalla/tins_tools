
function usage()
{
    echo "usage $0 <srtm30_path>"
    echo ""
}
if [ $# -eq 1 ]
then
    path="$1"
    find $path -type f | grep "hgt\.gz"  | while read full_path
    do
        name=$(echo "$full_path" | sed 's/\.gz//g')
        echo "---------------------------------"
        ls -lrth $full_path 
        # Unzip file 
        gunzip $full_path
        ls -lrth $name
        touch $name
        # Chmod to prevet some WSL Ubuntu permission errors
        chmod 0777 "$name"
        # Zipping back
        gzip "$name"
        ls -lrth $full_path 
    done
else
    usage
fi