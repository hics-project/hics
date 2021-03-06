/* Copyright 2018 The HICS Authors
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

#include <Manager.h>
#include "MainWindowSubjectTest.h"
#include "MainWindowSubject.h"

TEST_CASE("MainWindowSubject can attach, detach and notfiy ManagerObserver's", "[mainwindowsubject]") {

    MainWindowSubject subject;
    ManagerObserver observer1;
    subject.attach(&observer1);

    //Test attach
    REQUIRE(subject.getObservers().size() == 1);

    SECTION ("attach() test when one is already attached") {
        REQUIRE(subject.getObservers().size() == 1);

        ManagerObserver observer2;
        subject.attach(&observer2);
        REQUIRE(subject.getObservers().size() == 1);
    }

    SECTION("Method detach() test", "[detach]") {
        subject.detach(&observer1);
        REQUIRE(subject.getObservers().size() == 0);
    }

    SECTION("Method notify() test", "[notify]") {
        ClassificationResult *result = subject.notify();
        REQUIRE(result == nullptr);
    }
}
