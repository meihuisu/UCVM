#!/bin/bash

# make_pdf.sh - Create CVM-H User Guide PDF from Wiki page

# Filesnames
URL="http://scec.usc.edu/scecwiki/index.php?title=CVM-H_User_Guide&printable=yes"
DL_DIR="./scec.usc.edu"
URL_FILE="${DL_DIR}/scecwiki/index*"
HTML_FILE="${DL_DIR}/scecwiki/index.html"
HTML_TITLE="cvmh_manual_title.html"
PDF_FILE="../../doc/cvmh_manual.pdf"
TMP_FILE="tmp.html"
TMP_FILE2="tmp2.html"
SED_FILE="tmp.sed"

# Header/footer to replace saved html
DOC_HDR='<HTML><META content="text/html; charset=iso-8859-1" http-equiv=Content-Type><BODY>'
DOC_FTR="</BODY></HTML>"
DOC_DATE=`/bin/date -u`

# Check that revision id was specified
if [ $# -lt 1 ]; then
	printf "Usage: %s: <revision>\n" $(basename $0) >&2    
        exit 1
fi

# Retrieve HTML and associated images from wiki
rm -rf ${DL_DIR}
wget --recursive --page-requisites --accept=".jpg,.png,*CVM*" -k -np ${URL}

# Convert from UTF-8 to ISO-8859 encoding
mv ${URL_FILE} ${HTML_FILE}
iconv --from-code=UTF-8 --to-code=ISO-8859-1 ${HTML_FILE} > ${TMP_FILE}
cp ${TMP_FILE} ${HTML_FILE}

# Insert page break hints
sed -i "s/<pre>/<!-- NEED 12 --><pre>/g" ${HTML_FILE}
sed -i "s/<table/<!-- NEED 12 --><table/g" ${HTML_FILE}

# Determine start/end lines to cut out from HTML
START_LINE=`grep -n -m 1 "<a name=\"Overview\" id=\"Overview\"></a>" ${HTML_FILE} | awk -F ':' '{print $1}'`
END_LINE=`grep -n -m 1 "Saved in parser" ${HTML_FILE} | awk -F ':' '{print $1}'`

# Perform cutting
head -n ${END_LINE} ${HTML_FILE} > ${TMP_FILE}
echo "${DOC_FTR}" >> ${TMP_FILE}
echo "${DOC_HDR}" > ${TMP_FILE2}
tail -n +${START_LINE} ${TMP_FILE} >> ${TMP_FILE2}

cp ${TMP_FILE2} ${HTML_FILE}

# Replace date and revision in title
echo "s/REVISION/$1/g" > ${SED_FILE}
echo "s/DATE/${DOC_DATE}/g" >> ${SED_FILE}
sed -f ${SED_FILE} ${HTML_TITLE} > ${TMP_FILE}

# Generate PDF book
htmldoc --firstpage p1 --fontsize 10.0 --titlefile ${TMP_FILE} -f ${PDF_FILE} ${HTML_FILE}

rm ${SED_FILE}
rm ${TMP_FILE}
rm ${TMP_FILE2}
rm -rf ${DL_DIR}
