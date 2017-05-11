echo ""
echo "// @$1"
cat $1 | grep -v '^\t' | grep -v '{' | grep -v '}' | grep -v ';' \
	| sed 's/)$/);/g' | grep '^[a-zA-Z(]' | sed -e "s/^(/(/g" \
	| grep -v 'NODEID' | grep -v 'RELID' | grep -v 'Internal'
echo ""
