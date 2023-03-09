"use strict";
/* global describe it before */
/*
  This utility file is intended to run in a different process space
  than the main tests.  This helps to ensure that things do get
  written to disk properly.  Doing these tests in the same process
  space where the tests are written is problematic as Boost shares
  allocators.
*/

const binary = require("node-pre-gyp");
const path = require("path");
const sharedFileViewPath = binary.find(path.resolve(path.join(__dirname, "../package.json")));
const SharedFileView = require(sharedFileViewPath);
const expect = require("chai").expect;

const testFilePath = path.join(__dirname, "..", "testdata", "random_strings.txt");

describe("Instance functions from another process", function () {
	it("get the array", function () {
		const array = new SharedFileView.ArrayFrom(testFilePath);
		expect(array.length).to.equal(10);
	});

	it("gets first line", function () {
		const array = new SharedFileView.ArrayFrom(testFilePath);
		expect(array[0]).to.equal("6wjZagsBmTxY3xeMc4brVplX");
	});
	it("gets second line", function () {
		const array = new SharedFileView.ArrayFrom(testFilePath);
		expect(array[1]).to.equal("bWw4FBK0iKnncEPDcz7weEMwZpxy2gkNdx3lc");
	});
	it("gets last line", function () {
		const array = new SharedFileView.ArrayFrom(testFilePath);
		expect(array[array.length - 1]).to.equal("mvkref3kq5VwOcCusoeOD2KJN6poRYFmN5WCr");
	});
	it("gets a line before allowed range", function () {
		const array = new SharedFileView.ArrayFrom(testFilePath);
		expect(array[-1]).to.be.undefined;
	});
	it("gets a line after allowed range", function () {
		const array = new SharedFileView.ArrayFrom(testFilePath);
		expect(array[array.length]).to.be.undefined;
	});
});

describe("Static functions from another process", function () {
	it("it exists", function () {
		expect(SharedFileView.Exists(testFilePath)).to.be.true;
	});
	it("it removes", function (done) {
		SharedFileView.Remove(testFilePath, (err) => {
			expect(SharedFileView.Exists(testFilePath)).to.be.false;
			done(err);
		});
	});
});
