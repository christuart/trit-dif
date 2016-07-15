#!/bin/bash

echo
echo "************************************************************"
echo "***"
echo "***  process-msh.sh script written by Chris Stuart"
echo "***  for generating easy computer-readable"
echo "***  files from Gmsh outputs files."
echo "***"
echo "***  This version: 15/07/2016"
echo "***"
echo "***  .msh output from Gmsh is defined at "
echo "***  http://www.manpagez.com/info/gmsh/gmsh-2.2.6/gmsh_63.php"
echo "***"
echo "************************************************************"
echo

usage()
{
    echo "Usage: $0 [[[-f|--file filename ] [-i|--input ] [[-a|--append ] | [-d|--delete ]]] | [-h|--help ]]"
    echo
    echo $'-a --append\tAppend the discovered data to the output files, rather\n\t\tthan deleting them first'
    echo $'-d --delete\tDelete the old output files, rather than appending'
    echo $'-f --file\tSpecify the .msh file, with or without the extension'
    echo $'-h --help\tShow this usage information'
    echo $'-i --input\tProvide data at prompts in the script'
    echo
}

declare -i sectionscount=0
declare -i nodescount=0
declare -i elementscount=0

#Read in the command line arguments
file_name=
delete=
append=
input=
while [ "$1" != "" ]; do
    case $1 in
        -f | --file )
            shift
            file_name=$1
            ;;
        -d | --delete )
            if [ -z $append]
	    then
		delete=1
	    fi
            ;;
	-a | --append )   
	    delete=
	    append=1
	    ;;
	-i | --input )
	    input=1
	    ;;
        -h | --help )
           usage
           exit
           ;;
        * )
            usage
            exit 1
    esac
    shift
done

# Make sure that the proper inputs have been given, otherwise show
# usage information and exit
if [[ (-z $input ) && ( (-z $file_name ) || ( ( -z $append ) && ( -z $delete ) ) ) ]]
then
    usage
    exit 1
fi   

# Get the name of the .msh file, allowing either "name" or "name.msh"
# Only read if hasn't been provided as an argument
if [ -z $file_name ]
then
    echo "??? Name of .msh to process: "
    read file_name
fi
file_name=$(echo $file_name | sed 's/\([a-z]*\)\(.msh\)\{0,1\}$/\1.msh/')

echo "*** Using file: $file_name"
echo

# Make names of the output files
file_nodes=$(echo $file_name | sed 's/\([a-z]*\)\(.msh\)$/\1.nodes/')
file_elements=$(echo $file_name | sed 's/\([a-z]*\)\(.msh\)$/\1.elements/')
file_sections=$(echo $file_name | sed 's/\([a-z]*\)\(.msh\)$/\1.sections/')

# Check for overwriting
if [ -f $file_nodes ] || [ -f $file_elements ] || [ -f $file_sections ]
then
    # Check whether already specified to append or delete
    if [ -z $append]
    then
	if [ -z $delete ]
	then
            # Ask whether to delete these or not. I don't like deleting files without prompting the
            # user first. I can't actually foresee a time when you would want the data to be appended
            # to an old file but, hey whatever!
	    echo "??? Processed files are already present. Would you like to delete"
	    echo "??? these before continuing? [y/n]"
	    read -r -p "??? (If they remain, data will be appended.) " response
            # Put response to lower case
    	    response=${response,,}
            # Check that response is something valid, or ask again if not
	    while ! [[ $response =~ ^(yes|y|no|n)$ ]]
	    do
		read -r -p "??? Pardon? [y/n] " response
		response=${response,,}
	    done
            # if they answered yes or y, delete the files which exist
	    if [[ $response =~ ^(yes|y)$ ]]
	    then
		delete=1
	    fi
	fi
    fi
    if [ $delete ]
    then
	if [ -f $file_nodes ]
	then
	    echo "*** Deleting $file_nodes"
	    rm $file_nodes
	fi
	if [ -f $file_elements ]
	then
	    echo "*** Deleting $file_elements"
	    rm $file_elements
	fi
	if [ -f $file_sections ]
	then
	    echo "*** Deleting $file_sections"
	    rm $file_sections
	fi
    fi
fi
echo
while read line
do
    case $line in
	"\$MeshFormat")
	    echo "*** Found mesh format data!"
	    while [ "$line" != "\$EndMeshFormat" ]
	    do
		read version filetype datasize <<< "$line"
		read line
	    done
	    if [ "$version" == "2.2" ]
		then
		echo "*** .msh format version is 2.2 as expected"
		else
		echo "!!! .msh format version is $version, 2.2 was expected. Continuing..."
	    fi
	    echo
	    ;;
	"\$PhysicalNames")
	    echo "*** Found physical names..."
	    read line
	    if [ "$line" != "\$EndPhysicalNames" ]
	    then
		sectionscount=$line
		declare -i sectioni=0
		echo "*** Storing $sectionscount physical names in $file_sections"
                echo $sectionscount >> "./$file_sections"
                echo "(dimensions id name)" >> "./$file_sections"
		read line
		while [ "$line" != "\$EndPhysicalNames" ]
		do
		    sectioni=$sectioni+1
		    read dim id name <<< "$line"
		    if [ "$id" != "$sectioni" ]
		    then
			echo "!!! Physical names id mixup. File: $id; Script: $sectioni"
		    fi
		    echo $dim $id $name >> "./$file_sections"
		    read line
		done
		if [ "$sectionscount" != "$sectioni" ]
		then
		    echo "!!! Physical names count mixup. Expected: $sectionscount; Found: $sectioni"
		fi
	    else
		echo "!!! Physical names section empty. Strange..."
	    fi
	    echo
	    ;;
	"\$Nodes")
	    echo "*** Found nodes..."
	    read line
	    if [ "$line" != "\$EndNodes" ]
	    then
		nodescount=$line
		declare -i nodei=0
		echo "*** Storing $nodescount nodes in $file_nodes"
                echo $nodescount >> "./$file_nodes"
                echo "(id x y z)" >> "./$file_nodes"
		read line
		while [ "$line" != "\$EndNodes" ]
		do
		    nodei=$nodei+1
		    read id x y z <<< "$line"
		    if [ "$id" != "$nodei" ]
		    then
			echo "!!! Nodes id mixup. File: $id; Script: $nodei"
		    fi
		    echo $id $x $y $z >> "./$file_nodes"
		    read line
		done
	    else
		echo "!!! Nodes section empty. Strange..."
	    fi
	    echo
	    ;;
	"\$Elements")
	    echo "*** Found elements..."
	    read line
	    if [ "$line" != "\$EndElements" ]
	    then
		elementscount=$line
		declare -i elementi=0
		echo "*** Storing $elementscount elements in $file_elements"
                echo $elementscount >> "./$file_elements"
                echo "(id element-type #-of-tags tag1:physical-section-id <other tags> <nodes>)" >> "./$file_elements"
		read line
		while [ "$line" != "\$EndElements" ]
		do
		    elementi=$elementi+1
		    read id type tagdata <<< "$line"
		    if [ "$id" != "$elementi" ]
		    then
			echo "!!! Nodes id mixup. File: $id; Script: $elementi"
		    fi
		    # Now let's see if there are element types which haven't been accommodated yet in this .msh
		    # Currently accommodated are:
		    #     1 - linear bar element
		    #     (as of 04-07-2016)
		    if ! [[ "$type" =~ ^(1|2|1)$ ]] # ^(1|2|1)$ used here to demonstrate how multiple element types will be accommodated here, will be e.g. ^(1|2|5|17|16)$
		    then
			echo "!!! The element type $type is found in this mesh, but is not yet known to"
			echo "!!! be accommodated for, so you might need to adjust the mesh for the"
			echo "!!! diffusion model to work. (Found at element id $id.)"
		    fi
		    echo $id $type $tagdata >> "./$file_elements"
		    read line
		done
	    else
		echo "!!! Nodes section empty. Strange..."
	    fi
	    echo
	    ;;
	"\$Periodic")
	    echo "*** Found periodicity data..."
	    echo "!!! Periodicity data is not handled by this script"
	    while [ "$line" != "\$EndPeriodic" ]
	    do
		read line
	    done
	    echo
	    ;;
	"\$NodeData")
	    echo "*** Found node data..."
	    echo "!!! Node data is not handled by this script"
	    while [ "$line" != "\$EndNodeData" ]
	    do
		read line
	    done
	    echo
	    ;;
	"\$ElementData")
	    echo "*** Found element data..."
	    echo "!!! Element data is not handled by this script"
	    while [ "$line" != "\$EndElementData" ]
	    do
		read line
	    done
	    echo
	    ;;
	"\$ElementNodeData")
	    echo "*** Found element-node data..."
	    echo "!!! Element-node data is not handled by this script"
	    while [ "$line" != "\$EndElementNodeData" ]
	    do
		read line
	    done
	    ;;
	"\$InterpolationScheme")
	    echo "*** Found interpolation scheme data..."
	    echo "!!! Interpolation schemes are not handled by this script"
	    while [ "$line" != "\$EndInterpolationScheme" ]
	    do
		read line
	    done
	    echo
	    ;;
	*)
	    echo "!!! Unhandled line: $line"
	    ;;
    esac
done < "$file_name"
