/*
 * guestbook script.
 */
var guestbook = (function() {
    // endpoints
    var GUESTS_PATH = '/guests.log';

    // messages
    var NO_GUESTS_MESSAGE = "You are the first guest!";
    var LOAD_ERROR = "Error loading guests.";
    var SUBMIT_ERROR = "Error submitting guest information.";

    var fieldLabels = {
        location: 'current location',
        song: 'song stuck in my head',
        food: 'best thing i ate this week',
        artist: 'favourite artist',
        game: 'favourite video game',
        movie: 'favourite movie',
        secret: 'secret'
    };

    // make an async HTTP request
    function sendRequest(method, path, postData, onDone) {
        var request = new XMLHttpRequest();
        request.onreadystatechange = function() {
            if (request.readyState === request.DONE) {
                onDone(request);
            }
        };

        request.open(method, path, true);
        if (postData) {
            request.setRequestHeader('Content-Type', postData.contentType);
            request.send(postData.body);
        } else {
            request.send();
        }
    }

    // clean a user-provided string
    function sanitizeString(string) {
        return string.replace(/\s+/g, ' ').trim();
    }

    // run callback for each named field in a form
    function forEachField(form, callback) {
        var fields = form.querySelectorAll(
            'input[type="text"], input[type="password"], textarea');
        for (var i = 0, length = fields.length; i < length; ++i) {
            var field = fields[i];
            if (field.name) {
                callback(field);
            }
        }
    }

    // check a form field for content and apply a style if invalid
    function validateField(field) {
        field.classList.remove('invalid');
        return true;
    }

    // extract form field values into an object
    function serializeForm(form) {
        var object = {};
        forEachField(form, function(field) {
            var value = field.value.trim();
            if (value.length > 0) {
                object[field.name] = value;
            }
        });
        return object;
    }

    // parse the contents of the guests log into a list of guest objects
    function parseGuests(guestsString) {
        var guests = [];
        var guestStrings = guestsString.split('\n\n');
        for (var i = guestStrings.length - 1; i >= 0; --i) {
            try {
                var guestJson = guestStrings[i].trim();
                if (guestJson.length > 0) {
                    var guestObject = JSON.parse(guestJson);
                    guests.push(guestObject);
                }
                guests = guests.filter(function(guest) {
                    return typeof guest.timestamp == 'number';
                });
            } catch (exception) {
                if (!(exception instanceof SyntaxError)) {
                    throw exception;
                }
            }
        }
        return guests;
    }

    // replace reserved HTML entities with escapes
    function escapeHtml(string) {
        return string.replace(/[&<>"']/g, function(character) {
            return "&#" + character.charCodeAt(0) + ";";
        });
    }

    // convert a guest object into a log entry markup string
    function renderMessage(message) {
        return '<div class="entry">' + escapeHtml(message) + '</div>';
    }

    // convert a guest object into a log entry markup string
    function renderGuest(guest) {
        var name = escapeHtml(guest.name || 'anonymous');
        var date = (new Date(guest.timestamp)).toLocaleString();
        var rows = Object.keys(fieldLabels)
            .filter(function(key) { return guest[key]; })
            .map(function(key) {
                return '<p class="entry-message"><strong>' + fieldLabels[key] + ':</strong> '
                    + escapeHtml(guest[key]) + '</p>';
            })
            .join('');
        return '<article class="entry">'
                + '<header class="entry-info">'
                    + '<span class="entry-name">' + name + '</span> '
                    + '<span class="entry-date">' + date + '</span>'
                + '</header>'
                + rows
            + '</article>';
    }

    // load all guests and populate the guest log
    function loadGuests(log) {
        sendRequest('GET', GUESTS_PATH, null,
            function(request) {
                // populate log
                if (request.status === 200) {
                    var guests = parseGuests(request.response);

                    // show placeholder if there are no guests
                    if (guests.length === 0) {
                        log.innerHTML = renderMessage(NO_GUESTS_MESSAGE);
                    // render guests
                    } else {
                        log.innerHTML = guests.map(renderGuest).join('');
                    }

                // show error message
                } else {
                    log.innerHTML = renderMessage(LOAD_ERROR);
                }
            });
    }

    // submit the guest contained in the form and refresh the guest log
    function addGuest(form, log) {
        var guest = serializeForm(form);

        // build payload
        guest.timestamp = Date.now();
        if (guest.name) {
            guest.name = sanitizeString(guest.name);
        }
        var guestJson = JSON.stringify(guest);
        var postData = {
            contentType: "application/json",
            body: guestJson
        };

        // POST JSON to guests endpoint
        sendRequest('POST', GUESTS_PATH, postData,
            function(request) {
                // refresh the log and remove the form
                if (request.status === 200) {
                    loadGuests(log);
                    form.parentElement.removeChild(form);

                // show error message
                } else {
                    window.alert(SUBMIT_ERROR);
                }
            });
    }

    return {
        init: function(selector) {
            var guestbook = document.querySelector(selector);
            var form = guestbook.querySelector(".form");
            var submit = guestbook.querySelector(".submit");
            var log = guestbook.querySelector(".log");

            // register field validators
            forEachField(form, function(field) {
                field.addEventListener('input', function(event) {
                    validateField(field);
                });
            });

            // register submit handler
            submit.addEventListener('click', function(event) {
                addGuest(form, log);
            });

            // populate
            form.reset();
            loadGuests(log);
        }
    }
})();
