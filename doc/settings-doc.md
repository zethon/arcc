# Configuration Settings

**Note**: Several commands accept arguments that correspond a particular setting. **Arguments always take precedence.**

**`command.go.autolist`**<br/>
\- type: `boolean`</br>
\- default: `true`<br/>
\- relevant command: [`go`](go.md)<br/>
\- usage: Controls whether or no to list the items in a subreddit automatically after navigating into the subreddit.

 **`command.list.limt`**<br/>
\- type: `int`</br>
\- default: `5`<br/>
\- relevant command: [`list`](list.md)<br/>
\- usage: The number of items to list. 

**`command.list.type`**<br/>
\- type: `enum`</br>
\- possible values: 'new', 'hot', 'rising', 'controversial', 'top'<br/>
\- default: `hot`<br/>
\- relevant command: [`list`](list.md)<br/>
\- usage: The type of items to list. 

**`command.view.form`**<br/>
\- type: `enum`</br>
\- possible values: 'normal', 'mobile', 'compact', 'json'<br/>
\- default: `normal`<br/>
\- relevant command: [`view`](view.md)<br/>
\- usage: The type of page rendering to open in the browser. 
             
**`command.view.type`**<br/>
\- type: `enum`</br>
\- possible values: 'comments', 'url'<br/>
\- default: `url`<br/>
\- relevant command: [`view`](view.md)<br/>
\- usage: A setting of `url` will open the item's link, which may be a video, image, external link. `comments` will instead open the item's comment link. If the item is a self-post then `url` and `comments` are the same.

**`global.terminal.color`**<br/>
\- type: `boolean`</br>
\- default: `true`<br/>
\- usage: Enables or disables text color in the application.

**`render.list.name`**<br/>
\- type: `boolean`</br>
\- default: `false`<br/>
\- relevant command: [`list`](list.md)<br/>
\- usage: Print the item's [fullname](https://www.reddit.com/dev/api#fullnames)

**`render.list.title.length`**<br/>
\- type: `int`</br>
\- default: `0`<br/>
\- relevant command: [`list`](list.md)<br/>
\- usage: The maximum number of characters to print of an item's title. A value of `0` will print the entire title.

**`render.list.url`**<br/>
\- type: `boolean`</br>
\- default: `false`<br/>
\- relevant command: [`list`](list.md)<br/>
\- usage: Print the item's URL. If the item is a self-post then this is equivalent to the URL of the comments.

**`render.list.votes`**<br/>
\- type: `boolean`</br>
\- default: `true`<br/>
\- relevant command: [`list`](list.md)<br/>
\- usage: Print the item's upvote and downvote details.
