function navResponse() {
  let m = document.getElementById("nav-menu");
  m.className = m.className === "" ? "active" : "";

  let btn = document.getElementById("nav-toggle");
  btn.setAttribute("class", btn.getAttribute("class") === "" ? "active" : "");
}

function rb(min, max) {
  return Math.floor(Math.random() * (max - min + 1) + min);
}

let html = /* html */ `
<div class="nav-flex">
  <svg id="nav-toggle" class="" viewBox="0 0 100 100" width="2.25rem" height="2.25rem" fill="white">
    <rect y="10"  width="100" height="20" rx="10"></rect>
    <rect y="40" width="100" height="20" rx="10"></rect>
    <rect y="70" width="100" height="20" rx="10"></rect>
  </svg>
  <a class="nav-logo crazy-jonah" href="/">Jonah Sussman</a>
  <div id="nav-menu">
    <a class="nav-link" href="/">Home</a>
    <a class="nav-link" href="/about">About</a>
    <a class="nav-link" href="/projects">Projects</a>
    <a class="nav-link" href="/assets/Jonah%20Sussman%20Resume%20ONLINE%202021-02-18.pdf">Resume</a>
  </div>
</div>
`;

class JNav extends HTMLElement {
  constructor() {
    super();
    this.className = "sw-default-dark";
    this.id = "navbar";
    this.innerHTML = html;

    document.getElementById("nav-toggle").addEventListener("click", navResponse, false);
  }
}

customElements.define('j-nav', JNav);