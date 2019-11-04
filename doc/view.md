# `view`

Open a specified item in the default browser.

### Usage
`view <index> [-cu] [--normal|--mobile|--compact|--json]

### Options
`-c`,`--comments`   Open the item's comments
`-u`,`--url`        Open the url of item if the item is a link. Otherwise this will open the comments. 

`--normal`          Opens the standard URL of the item
`--mobile`          Opens the (old) mobile interface of the item
`--compact`         Opens the (new) compact mobile interface of the item
`--json`            Returns the JSON output of the item

### Notes
The `<index>` argument is the index of the item listed in the previous `list` command's results.