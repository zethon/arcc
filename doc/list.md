# `list`

List

### Usage
`list (new|hot|rising|contreversial|top) [--limit=<count>] [--sub=<subreddit>]`

### Options
`--limit=<count>` - Number of topics to list [default: 5]<br/>
`--sub=<subreddit>` - List items in <subreddit> [default: current sub]

### Settings
`command.list.type` - Default listing type, one of `new`, `hot`, `rising`, `contreversial`, `top`<br/>
`command.list.limit` - Default for the `limit` parameter

### Notes
The default number of topics can be changed using `list.limit.default`. For example: `set list.limit.default=10`.

