"use strict";
/*
  This is intended to run in a different process space than the main
  tests. Its job is to do enough stuff that a GC is very likely. This
  test is to guard against the GC causing the background close to lose
  access to resources it needs.
*/

const binary = require("node-pre-gyp");
const path = require("path");
const sharedFileViewPath = binary.find(path.resolve(path.join(__dirname, "../package.json")));
const SharedFileView = require(sharedFileViewPath);
const expect = require("chai").expect;
const async = require("async");

function closeit(done) {
	const reader = new SharedFileView.Open(process.env.TESTFILE);
	expect(reader.first).to.equal("value for first");
	reader.close(done);
}

async.times(4000, function (n, next) {
	process.nextTick(closeit, next);
});
