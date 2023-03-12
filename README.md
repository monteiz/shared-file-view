# Shared File View

![Tests passing](https://github.com/monteiz/shared-file-view/actions/workflows/test.yml/badge.svg)
[![NPM version](https://badge.fury.io/js/shared-file-view.svg)](https://badge.fury.io/js/shared-file-view)

SharedFileView is a Node.js add-on that loads a file into a shared memory segment, **allowing multiple processes to access the same memory data** quickly and efficiently, as a JavaScript array. The returned array is read-only.

SharedFileView is designed for speed and minimal memory usage. By using shared memory, SharedFileView eliminates the need to copy data between processes, resulting in blazing fast performance. Additionally, SharedFileView uses the minimum memory allocation possible to load the text file into memory, ensuring that your system resources are used efficiently.

## Requirements

Boost libraries (^1.81) installed on the system. Installation will fail otherwise.

Follow [this guide](https://www.boost.org/doc/libs/1_81_0/more/getting_started/index.html) to install it.

## Installation

```npm
npm install shared-file-view
```

## Usage

### Creating a SharedFileView

First create a SharedFileView for a file, using the `SharedFileView.Create` static method. This method is asynchronous, so you can pass a callback function as the second argument to be called when the SharedFileView is actually created.

```js
const { SharedFileView } = require("shared-file-view");

SharedFileView.Create("/path/to/file.txt", (err) => {
	if (err) {
		console.error(err);
	} else {
		console.log("SharedFileView created");
	}
});
```

### Reading from an existing SharedFileView

To retrieve an array of lines from a file, use the `SharedFileView.ArrayFrom` constructor. This method returns a JavaScript array that you can use to access any line of the file.

```js
const { SharedFileView } = require("shared-file-view");

const filePath = "/path/to/file.txt";
const sharedFileView = new SharedFileView.ArrayFrom(filePath);

console.log(sharedFileView[0]); // Prints the first line of the file
console.log(sharedFileView[1]); // Prints the second line of the file
```

### Checking if a SharedFileView Exists

To check if a SharedFileView for a file has already been created, use the `SharedFileView.Exists` static method. This method returns a boolean indicating whether the SharedFileView exists.

```js
const { SharedFileView } = require("shared-file-view");

const filePath = "/path/to/file.txt";
const exists = SharedFileView.Exists(filePath);

console.log(exists); // Prints true if a SharedFileView exists, false otherwise
```

### Removing a SharedFileView

To remove a SharedFileView from memory and free up system resources, use the `SharedFileView.Remove` static method.

```js
const { SharedFileView } = require("shared-file-view");

const filePath = "/path/to/file.txt";
SharedFileView.Remove(filePath, (err) => {
	if (err) {
		console.error(err);
	} else {
		console.log("SharedFileView removed");
	}
});
```

## License

SharedFileView is licensed under the [MIT License](https://opensource.org/licenses/MIT). See the [`LICENSE`](LICENSE) file for details.

## Credits

I would like to thank the following individuals for their contributions to SharedFileView:

- Seth Heeren ([@sehe](https://github.com/sehe)), for providing the original approach that underlie SharedFileView.
- Allen Luce ([@allenluce](https://github.com/allenluce)), for creating the [`mmap-object`](https://github.com/allenluce/mmap-object) package that inspired SharedFileView.

Without the the contributions of Seth Heeren and the pioneering work of Allen Luce, SharedFileView would not be possible. I am grateful for their efforts and for making their work available under the MIT License.
