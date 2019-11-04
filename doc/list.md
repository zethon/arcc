# `list`

List the content of the current subreddit. If there is no active subreddit, then the list is equivalent to users home listing.

### Usage
`list (new|hot|rising|contreversial|top) [--limit=<count>] [--sub=<subreddit>]`

### Options
`--limit=<count>` - Number of topics to list [default: 5]<br/>
`--sub=<subreddit>` - list items in `<subreddit>` [default: current sub]
`-t <top>` - used with `list top`, one of `hour`, `day`, `week`, `month`, `year`, `all` 

### Settings
`command.list.type` - Default listing type, one of `new`, `hot`, `rising`, `contreversial`, `top`<br/>
`command.list.limit` - Default for the `limit` parameter

### Notes
The default number of topics can be changed using `list.limit.default`. For example: `set list.limit.default=10`.

