LibParagraph
============

Library for performing text layout.

TODO
----

### Content management

#### Notes

* Add way to remove content from a paragraph.
  - How to identify which content to remove? Client handle? Would need
    to loop through entire paragraph's content entries to find a match
    for the handle.
    - Otherwise change the API to return an opaque handle for the paragraph
      content entry to the client, in each of the content_add functions.
  - How to handle removing an inline start or inline end?
    - Maybe removing the start removes all content up to and including matching
      end?
    - Maybe removing an end is an error.
* Add way to modify a content style.
  - Again; it will help for the client to have a handle for the paragraph
    content entry, so it can say what it's changing the style of.
* Add way to insert after/before some existing content entry. At the moment
  it only supports append to end of paragraph.

#### Actions

* Change content add functions to return a content entry pointer for the
  new content entry.
* Change API to take an existing entry to insert before/after.
  - If NULL, insert at end.
  - Add enum to specify before/after.
  - Consolidate content add functions into one with parameters struct?

### Ownership

#### Notes

* Client string: Do we need a way to ref/unref? Or is assuming a client keeps
  it around while we might use it safe?
* Client styles: Do we need a way to ref/unref? Or is assuming a client keeps
  it around while we might use it safe?

### Styles

#### Notes

* Need a way to get values from the opaque client style.
  - Probably match callback functions to libcss computed style API.
* How to handle multiple styles per content element?
  - E.g. hover visited, etc?
  - Let the client handle it all by changing style on the content entry?
  - Who is responsible for splitting for stuff like first-letter, first-word,
    etc? Us or client?

### Text

#### Notes

* Going to need to split text into runs efficiently, for at least:
  - Bidirectional algorithm.
  - Styles (first-letter, first-word, etc)
  - Line break
  - Full justification?
