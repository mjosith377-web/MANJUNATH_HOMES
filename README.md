# MANJUNATH_HOMES
# Haven rental rooms demo

A static webpage based on the rental room management program. It includes tenant registration, room occupancy, rent payment proof details, owner review actions, and property rules.

## Open it locally

Open `index.html` in a modern browser.

## Publish with GitHub Pages
 
1. Create a GitHub repository and add `index.html` and this `README.md` to the repository's root.
2. In the repository, open **Settings → Pages**.
3. Under **Build and deployment**, choose **Deploy from a branch**, select the default branch and the `/ (root)` folder, then save.
4. GitHub will show the published page address in the Pages settings after deployment finishes.

## Demo storage

This is a front-end only demo. Tenant data is saved in the current browser's `localStorage`; it is not sent to GitHub and is not shared between visitors or devices. The owner view has a password prompt, but GitHub Pages serves the page code publicly, so this is only a casual-access screen and is not real authentication. The payment form records a screenshot filename only. It does not upload an image. Do not use this static demo to store real tenant personal or payment information.
